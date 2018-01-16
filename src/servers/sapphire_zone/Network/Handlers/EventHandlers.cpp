#include <common/Common.h>
#include <common/Exd/ExdData.h>
#include <common/Network/CommonNetwork.h>
#include <common/Network/GamePacketNew.h>
#include <common/Network/PacketContainer.h>
#include <common/Network/PacketDef/Zone/ServerZoneDef.h>

#include <boost/format.hpp>

#include "Network/GameConnection.h"
#include "Session.h"
#include "Network/PacketWrappers/ServerNoticePacket.h"
#include "Network/PacketWrappers/ActorControlPacket142.h"
#include "Network/PacketWrappers/ActorControlPacket143.h"
#include "Network/PacketWrappers/ActorControlPacket144.h"
#include "Network/PacketWrappers/EventStartPacket.h"
#include "Network/PacketWrappers/EventFinishPacket.h"
#include "Network/PacketWrappers/PlayerStateFlagsPacket.h"
#include "Script/ScriptManager.h"
#include "Actor/Player.h"
#include "Forwards.h"
#include "Event/EventHelper.h"

extern Core::Data::ExdData g_exdData;
extern Core::Scripting::ScriptManager g_scriptMgr;

using namespace Core::Common;
using namespace Core::Network::Packets;
using namespace Core::Network::Packets::Server;

void Core::Network::GameConnection::eventHandler( const Packets::GamePacket& inPacket,
                                                  Entity::Player& player )
{
   uint16_t eventHandlerId = inPacket.getValAt< uint16_t >( 0x12 );

   // we need to abort the event in case it has not been scripted so the player wont be locked up
   auto abortEventFunc = []( Core::Entity::Player& player, uint64_t actorId, uint32_t eventId )
   {
      player.queuePacket( EventStartPacket( player.getId(), actorId, eventId, 1, 0, 0 ) );
      player.queuePacket( EventFinishPacket( player.getId(), eventId, 1, 0 ) );
      // this isn't ideal as it will also reset any other status that might be active
      player.queuePacket( PlayerStateFlagsPacket( player, PlayerStateFlagList{} ) );
   };

   std::string eventIdStr = boost::str( boost::format( "%|04X|" ) % static_cast< uint32_t >( eventHandlerId & 0xFFFF ) );
   player.sendDebug( "---------------------------------------" );
   player.sendDebug( "EventHandler ( " + eventIdStr + " )" );

   switch( eventHandlerId )
   {
   
   case ClientZoneIpcType::TalkEventHandler: // Talk event
   case ClientZoneIpcType::EmoteEventHandler: // Emote event
   case ClientZoneIpcType::WithinRangeEventHandler:
   case ClientZoneIpcType::OutOfRangeEventHandler:
   case ClientZoneIpcType::EnterTeriEventHandler:
      break;

   case ClientZoneIpcType::ReturnEventHandler:
   case ClientZoneIpcType::TradeReturnEventHandler:
   {
      uint32_t eventId = inPacket.getValAt< uint32_t >( 0x20 );
      uint16_t subEvent = inPacket.getValAt< uint16_t >( 0x24 );
      uint16_t param1 = inPacket.getValAt< uint16_t >( 0x26 );
      uint16_t param2 = inPacket.getValAt< uint16_t >( 0x28 );
      uint16_t param3 = inPacket.getValAt< uint16_t >( 0x2C );

      std::string eventName = Event::getEventName( eventId );

      if( !g_scriptMgr.onEventHandlerReturn( player, eventId, subEvent, param1, param2, param3 ) )
         abortEventFunc( player, 0, eventId );
      break;
   }

   case ClientZoneIpcType::LinkshellEventHandler:
   case ClientZoneIpcType::LinkshellEventHandler1:
   {
      uint32_t eventId = inPacket.getValAt< uint32_t >( 0x20 );
      uint16_t subEvent = inPacket.getValAt< uint16_t >( 0x24 );
      std::string lsName = inPacket.getStringAt( 0x27 );

      ZoneChannelPacket< FFXIVIpcEventLinkshell > linkshellEvent( player.getId() );
      linkshellEvent.data().eventId = eventId;
      linkshellEvent.data().scene = static_cast< uint8_t >( subEvent );
      linkshellEvent.data().param3 = 1;
      linkshellEvent.data().unknown1 = 0x15a;
      player.queuePacket( linkshellEvent );

//      abortEventFunc( pPlayer, 0, eventId );
      break;
   }

   }

}

void Core::Network::GameConnection::eventHandlerTalk( const Packets::GamePacket& inPacket, Entity::Player& player )
{

   auto actorId = inPacket.getValAt< uint64_t >( 0x20 );
   auto eventId = inPacket.getValAt< uint32_t >( 0x28 );
   auto eventType = static_cast< uint16_t >( eventId >> 16 );

   std::string eventName = "onTalk";
   std::string objName = Event::getEventName( eventId );

   player.sendDebug( "Actor: " +
                     std::to_string( actorId ) + " -> " +
                     std::to_string( Event::mapEventActorToRealActor( static_cast< uint32_t >( actorId ) ) ) +
                     " \neventId: " +
                     std::to_string( eventId ) +
                     " (0x" + boost::str( boost::format( "%|08X|" )
                                          % static_cast< uint64_t >( eventId & 0xFFFFFFF ) ) + ")" );


   player.sendDebug( "Calling: " + objName + "." + eventName );
   player.eventStart( actorId, eventId, Event::EventHandler::Talk, 0, 0 );

   if( !g_scriptMgr.onTalk( player, actorId, eventId ) &&
       eventType == Event::EventHandler::EventHandlerType::Quest )
   {
      auto questInfo = g_exdData.getQuestInfo( eventId );
      if ( questInfo )
         player.sendUrgent( "Quest not implemented: " + questInfo->name + " (" + questInfo->name_intern + ")" );
   }

   player.checkEvent( eventId );

}

void Core::Network::GameConnection::eventHandlerEmote( const Packets::GamePacket& inPacket, Entity::Player& player )
{

   auto actorId = inPacket.getValAt< uint64_t >( 0x20 );
   auto eventId = inPacket.getValAt< uint32_t >( 0x28 );
   auto emoteId = inPacket.getValAt< uint16_t >( 0x2C );
   auto eventType = static_cast< uint16_t >( eventId >> 16 );

   std::string eventName = "onEmote";
   std::string objName = Event::getEventName( eventId );

   player.sendDebug( "Actor: " +
                     std::to_string( actorId ) + " -> " +
                     std::to_string( Event::mapEventActorToRealActor( static_cast< uint32_t >( actorId ) ) ) +
                     " \neventId: " +
                     std::to_string( eventId ) +
                     " (0x" + boost::str( boost::format( "%|08X|" )
                                          % static_cast< uint64_t >( eventId & 0xFFFFFFF ) ) + ")" );

   player.sendDebug( "Calling: " + objName + "." + eventName );

   player.eventStart( actorId, eventId, Event::EventHandler::Emote, 0, emoteId );

   if( !g_scriptMgr.onEmote( player, actorId, eventId, static_cast< uint8_t >( emoteId ) )  &&
       eventType == Event::EventHandler::EventHandlerType::Quest )
   {
      auto questInfo = g_exdData.getQuestInfo( eventId );
      if( questInfo )
         player.sendUrgent( "Quest not implemented: " + questInfo->name );
   }

   player.checkEvent( eventId );
}

void Core::Network::GameConnection::eventHandlerWithinRange( const Packets::GamePacket& inPacket,
                                                             Entity::Player& player )
{

   auto eventId = inPacket.getValAt< uint32_t >( 0x24 );
   auto param1 = inPacket.getValAt< uint32_t >( 0x20 );
   auto x = inPacket.getValAt< float >( 0x28 );
   auto y = inPacket.getValAt< float >( 0x2C );
   auto z = inPacket.getValAt< float >( 0x30 );

   std::string eventName = "onWithinRange";
   std::string objName = Event::getEventName( eventId );
   player.sendDebug( "Calling: " + objName + "." + eventName + " - " + std::to_string( eventId ) +
                     " p1: " + std::to_string( param1 ) );

   player.eventStart( player.getId(), eventId, Event::EventHandler::WithinRange, 1, param1 );

   g_scriptMgr.onWithinRange( player, eventId, param1, x, y, z );

   player.checkEvent( eventId );
}

void Core::Network::GameConnection::eventHandlerOutsideRange( const Packets::GamePacket& inPacket,
                                                              Entity::Player& player )
{

   auto eventId = inPacket.getValAt< uint32_t >( 0x24 );
   auto param1 = inPacket.getValAt< uint32_t >( 0x20 );
   auto x = inPacket.getValAt< float >( 0x28 );
   auto y = inPacket.getValAt< float >( 0x2C );
   auto z = inPacket.getValAt< float >( 0x30 );

   std::string eventName = "onOutsideRange";
   std::string objName = Event::getEventName( eventId );
   player.sendDebug( "Calling: " + objName + "." + eventName + " - " + std::to_string( eventId ) +
                     " p1: " + std::to_string( param1 ) );

   player.eventStart( player.getId(), eventId, Event::EventHandler::WithinRange, 1, param1 );

   g_scriptMgr.onOutsideRange( player, eventId, param1, x, y, z );

   player.checkEvent( eventId );
}

void Core::Network::GameConnection::eventHandlerEnterTerritory( const Packets::GamePacket &inPacket,
                                                                Entity::Player &player )
{
   auto eventId = inPacket.getValAt< uint32_t >( 0x20 );
   auto param1 = inPacket.getValAt< uint16_t >( 0x24 );
   auto param2 = inPacket.getValAt< uint16_t >( 0x26 );

   std::string eventName = Event::getEventName( eventId );

   std::string objName = Event::getEventName( eventId );

   player.sendDebug( "Calling: " + objName + "." + eventName + " - " + std::to_string( eventId ) );

   player.eventStart( player.getId(), eventId, Event::EventHandler::EnterTerritory, 0, player.getZoneId() );

   g_scriptMgr.onEnterTerritory( player, eventId, param1, param2 );

   player.checkEvent( eventId );
}


