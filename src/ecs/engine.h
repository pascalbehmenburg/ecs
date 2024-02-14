#ifndef ENGINE_H_
#define ENGINE_H_

#include <memory>

#include "component_manager.h"
#include "entity_manager.h"
#include "system_manager.h"
#include "type_def.h"

namespace ecs {
/// @file
/// @brief Engine provides an api (a facade) to the ECS.
class Engine final {
 public:
  /// @brief Retrieves a reference to the singleton engine object.
  /// @note The pattern was retrieved from:
  /// https://webkit.org/code-style-guidelines/#singleton-pattern
  /// Also see:
  /// https://codereview.stackexchange.com/questions/173929/modern-c-singleton-template
  /// @returns `Engine&` A reference to the singleton engine object.
  static Engine& singleton() {
    static Engine instance = Engine();
    return instance;
  }

  /// Ensure that the engine cannot be copied or moved.
  Engine(const Engine&) = delete;

  /// Ensure that the engine cannot be copied or moved.
  Engine& operator=(const Engine&) = delete;

  /// Ensure that the engine cannot be copied or moved.
  Engine(Engine&&) = delete;

  /// Ensure that the engine cannot be copied or moved.
  Engine& operator=(Engine&&) = delete;

  /// @brief Creates an entity and returns it.
  /// @returns `Entity` The created entity.
  Entity CreateEntity() { return entity_manager_->CreateEntity(); }

  /// @brief Destroys the provided entity thereby ensuring that no associated
  /// components or references in systems are left.
  /// @param entity The entity which should be destroyed.
  void DestroyEntity(const Entity entity) {
    entity_manager_->DestroyEntity(entity);
    component_manager_->EntityWasDestroyed(entity);
    system_manager_->EntityWasDestroyed(entity);

    spdlog::debug("Engine destroyed entity {}.", entity);
  }

  /// @deprecated Just use AddComponent(const Entity entity, const T component)
  /// instead.
  /// @brief Registers a component type so that it can be added to entities
  /// afterwards.
  template <typename T>
  void RegisterComponent() {
    component_manager_->RegisterComponent<T>();

    spdlog::debug("Engine registered component {}.", typeid(T).name());
  }

  /// @brief Associates a component of type T to the provided entity thereby
  /// notifying all systems about the change so that they may use or discard the
  /// entity based on the new compatibility constraints.
  /// @param entity The entity to which the component should be added.
  /// @param component The component which should be associated to the entity.
  template <typename T>
  void AddComponent(const Entity entity, const T component) {
    component_manager_->AddComponent<T>(entity, component);

    const auto component_id = component_manager_->GetComponentType<T>();

    auto entity_signature = entity_manager_->GetSignature(entity);
    entity_signature.set(component_id, true);
    entity_manager_->SetSignature(entity, entity_signature);

    system_manager_->EntitySignatureChanged(entity, entity_signature);

    spdlog::debug("Engine associated component {} to {}.", typeid(T).name(), entity);
  };

  /// @brief Removes the association between component of type T and the
  /// provided entity. Also it notifies all systems about the change so that
  /// they may use or discard the entity based on the new compatibility
  /// constraints.
  /// @param entity The entity from which the component should be removed.
  template <typename T>
  void RemoveComponent(const Entity entity) {
    component_manager_->RemoveComponent<T>(entity);

    const auto component_id = component_manager_->GetComponentType<T>();

    auto entity_signature = entity_manager_->GetSignature(entity);
    entity_signature.set(component_id, false);
    entity_manager_->SetSignature(entity, entity_signature);

    system_manager_->EntitySignatureChanged(entity, entity_signature);

    spdlog::debug("Engine removed component {} from {}.", typeid(T).name(), entity);
  }

  /// @brief Determines whether the component of type T is associated to the
  /// given entity.
  /// @param entity The entity for which the component should be associated to.
  /// @returns `bool` True if the entity has that component associated to it.
  template <typename T>
  bool HasComponent(const Entity entity) {
    auto component_signature = Signature{};
    component_signature.set(component_manager_->GetComponentType<T>(), true);

    const auto entity_signature = entity_manager_->GetSignature(entity);

    bool compatible = ((component_signature & entity_signature) == component_signature);

    spdlog::debug(
        "Engine checked if entity {} is associated to component {}. "
        "Result: {}.",
        entity, typeid(T).name(), compatible);

    return compatible;
  }

  /// @brief Returns the component of type T which is associated to the
  /// provided entity.
  /// @attention This method throws an exception if there is no such component
  /// associated to the entity.
  /// @param entity The entity for which the component should be returned.
  /// @returns `T&` The component of type T which is associated to the entity.
  template <typename T>
  T& GetComponent(const Entity entity) {
    spdlog::debug("Engine retrieves component {} associated to entity {}.", typeid(T).name(),
                  entity);
    return component_manager_->GetComponent<T>(entity);
  }

  /// @brief Returns the `ComponentType` of given component type T which is
  /// basically an id so that it can be used on signatures.
  /// @returns `ComponentType` The component type of type T.
  template <typename T>
  ComponentType GetComponentType() {
    ComponentType component_type = component_manager_->GetComponentType<T>();
    spdlog::debug(
        "Engine retrieved ComponentType (runtime uid) {} for component of type "
        "{}.",
        component_type, typeid(T).name());
    return component_type;
  }

  /// @brief Registers an already existing system of type T.
  /// @note This enables you to create systems with non-default constructor.
  /// @param system The system which was already constructed and should be
  /// registered.
  /// @returns `std::shared_ptr<T>` The system of type T.
  template <typename T>
  std::shared_ptr<T> RegisterSystem(T system) {
    spdlog::debug("Engine registers system {}.", typeid(T).name());
    return system_manager_->RegisterSystem<T>(system);
  }

  /// @brief Registers a system of type T.
  /// @attention If your system uses a non-default constructor consider using
  /// the RegisterSystem(T system) method instead.
  /// @returns `ComponentType` The component type of type T.
  template <typename T>
  std::shared_ptr<T> RegisterSystem() {
    spdlog::debug("Engine registers system {}.", typeid(T).name());
    return system_manager_->RegisterSystem<T>();
  }

  /// @brief Sets the signature of the system of type T to enable compatibility
  /// checks and provide it with corresponding entities.
  /// @param signature The signature which should be set for the system of type
  /// T.
  template <typename T>
  void SetSystemSignature(const Signature signature) {
    system_manager_->SetSystemSignature<T>(signature);
    spdlog::debug("Engine persisted signature {} for system {}.", signature.to_string(),
                  typeid(T).name());
  }

  /// @brief Returns the system of type T to be used in other system.
  /// @returns `std::shared_ptr<T>` The system of type T.
  template <typename T>
  std::shared_ptr<T> GetSystem() {
    spdlog::debug("Engine retrieves system {}.", typeid(T).name());
    return system_manager_->GetSystem<T>();
  }

 private:
  const std::unique_ptr<EntityManager> entity_manager_;
  const std::unique_ptr<ComponentManager> component_manager_;
  const std::unique_ptr<SystemManager> system_manager_;

  Engine()
      : entity_manager_(std::make_unique<EntityManager>()),
        component_manager_(std::make_unique<ComponentManager>()),
        system_manager_(std::make_unique<SystemManager>()) {
    spdlog::debug("Engine singleton instance created.");
  }
};
}  // namespace ecs

#endif  // ENGINE_H_