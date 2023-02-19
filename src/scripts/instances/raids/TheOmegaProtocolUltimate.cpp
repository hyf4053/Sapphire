#include <ScriptObject.h>
#include <Territory/InstanceContent.h>
#include <Manager/RNGMgr.h>

#include <Actor/EventObject.h>
#include <Actor/Player.h>
#include <Actor/BNpc.h>

#include <ServerMgr.h> //aka. WorldServer.h

#include <Network/GamePacket.h>
#include <Network/GameConnection.h>
#include <Network/PacketDef/Zone/ServerZoneDef.h>

using namespace Sapphire;
using namespace Sapphire::Network::Packets;

class TheOmegaProtocolUltimate :
  public Sapphire::ScriptAPI::InstanceContentScript
{
private:
  enum Omegas : int
  {
    Omega = -1,
    OmegaM = -1,
    OmegaF = -1,
    OpticalUnit = -1
  };

  enum Sequence : uint8_t
  {
    Seq1 = 1,
    Seq2 = 3,
    Seq3 = 7,
    Seq4 = 23,
    Seq5 = 31,
    SeqFinish = 255
  };

public:
  TheOmegaProtocolUltimate():
    Sapphire::ScriptAPI::InstanceContentScript(30124)
  {}

  void onInit(InstanceContent& instance) override
  {
   
  }

  void onUpdate(InstanceContent& instance, uint64_t tickCount) override
  {

  }

  void onEnterTerritory(InstanceContent& instance, Entity::Player& player, uint32_t eventId, uint16_t param1,
    uint16_t param2) override
  {

  }
};

EXPOSE_SCRIPT(TheOmegaProtocolUltimate);