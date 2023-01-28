#include <ScriptObject.h>
#include <Territory/InstanceContent.h>

using namespace Sapphire;

class TheOmegaProtocolUltimate :
  public Sapphire::ScriptAPI::InstanceContentScript
{
  TheOmegaProtocolUltimate():
    Sapphire::ScriptAPI::InstanceContentScript( 30124 )
  {}

  void onInit(InstanceContent& instance) override
  {
    instance.registerEObj("Exit", 2000139, 0, 4, { 0.000000f, 0.000000f, -20.000000f }, 1.000000f, 0.000000f);
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