#include <spdlog/cfg/env.h>

#include <SFML/Graphics.hpp>

#include "ecs/engine.h"

constexpr int window_width = 512;
constexpr int window_height = 512;
constexpr char window_title[] = "ECS Game";

int main() {
  spdlog::cfg::load_env_levels();

  auto engine = &ecs::Engine::singleton();

  auto entity = engine->CreateEntity();
  spdlog::info("Created entity {}.", entity);

  engine->RegisterComponent<float>();

  engine->AddComponent<float>(entity, 1.0);

  auto component_type = engine->GetComponentType<float>();
  spdlog::info("component_type: {}", component_type);

  spdlog::info("Added component to entity {}.", engine->GetComponent<float>(entity));

  auto has_component = engine->HasComponent<float>(entity);
  spdlog::info("Entity has component: {}", has_component);

  class TestSystem : public ecs::System {
   public:
    void Update() override {
      for (auto entity : entities_) {
        spdlog::info("TestSystem update Entity {}.", entity);
      }
    }
  };

  auto test_system = engine->RegisterSystem<TestSystem>();

  auto test_system_ptr = engine->GetSystem<TestSystem>();

  auto sig = ecs::Signature{};

  sig.set(engine->GetComponentType<float>(), true);

  engine->SetSystemSignature<TestSystem>(sig);

  test_system->Update();

  engine->RemoveComponent<float>(entity);

  test_system->Update();

  engine->DestroyEntity(entity);

  test_system->Update();

  return 0;
}
