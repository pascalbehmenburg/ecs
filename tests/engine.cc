#include "ecs/engine.h"

#include <gtest/gtest.h>

#include "ecs/entity_manager.h"
#include "ecs/system.h"

TEST(ECS, IntegrationTest) {
  auto engine = &ecs::Engine::singleton();

  ASSERT_NO_THROW(engine->CreateEntity());
  auto entity = engine->CreateEntity();

  ASSERT_NO_THROW(engine->RegisterComponent<float>());
  ASSERT_NO_THROW(engine->AddComponent<float>(entity, 1.0));
  ASSERT_NO_THROW(engine->GetComponentType<float>());
  ASSERT_NO_THROW(engine->GetComponent<float>(entity));
  ASSERT_NO_THROW(engine->HasComponent<float>(entity));

  class TestSystem : public ecs::System {
   public:
    void Update() {
      for (auto entity : entities_) {
        spdlog::info("TestSystem update Entity {}.", entity);
      }
    }
  };

  ASSERT_NO_THROW(engine->RegisterSystem<TestSystem>());

  ASSERT_NO_THROW(engine->GetSystem<TestSystem>());
  std::shared_ptr<TestSystem> test_system = engine->GetSystem<TestSystem>();

  auto sig = ecs::Signature{};
  sig.set(engine->GetComponentType<float>(), true);
  ASSERT_NO_THROW(engine->SetSystemSignature<TestSystem>(sig));

  ASSERT_NO_THROW(test_system->Update());

  ASSERT_NO_THROW(engine->RemoveComponent<float>(entity));

  ASSERT_NO_THROW(test_system->Update());

  ASSERT_NO_THROW(engine->DestroyEntity(entity));

  ASSERT_NO_THROW(test_system->Update());

  spdlog::info("Tested whole engine without any exceptions being thrown.");
}
