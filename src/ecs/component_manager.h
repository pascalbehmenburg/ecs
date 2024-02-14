#ifndef COMPONENT_MANAGER_H_
#define COMPONENT_MANAGER_H_

#include <spdlog/spdlog.h>

#include <map>

#include "component_list.h"
#include "type_def.h"

namespace ecs {
/// @file
/// @brief The component manager manages all components that ever exist within
/// the ECS and exposes methods for the Engine to interact on them.
/// @details For each new component type which is associated to the system using
/// the register method it creates a mapping from that very type to a singular
/// list instance for that component type and maps the actual type to an id for
/// internal usage.
class ComponentManager final {
 public:
  /// @brief Registers a component type.
  /// @details It's required before adding it to an entity so the
  /// component manager knows how to handle it and is able to allocate memory
  /// for the list of components of the given type.
  /// @attention Throws an exception if the component type is already
  /// registered.
  template <typename T>
  void RegisterComponent() {
    const char* component_type_name = typeid(T).name();

    if (component_types_.contains(component_type_name)) {
      spdlog::warn(
          "Tried to register component {} which is already registered. "
          "Consider removing RegisterComponent calls and use AddComponent "
          "directly instead.",
          component_type_name);
      return;
    }

    component_lists_[component_type_name] = std::make_shared<ComponentList<T>>();
    component_types_[component_type_name] = next_component_type_;
    next_component_type_++;
    spdlog::debug("Component manager registered component {}.", component_type_name);
  }

  /// @brief Exposes the component list of type T to the engine so that it can
  /// add components of type T to entities.
  /// @attention Registers the component type if it wasn't registered before.
  /// @param entity The entity to which the component should be added.
  /// @param component The component which should be added to the entity.
  template <typename T>
  void AddComponent(const Entity entity, const T component) {
    const char* component_type_name = typeid(T).name();

    if (!component_types_.contains(component_type_name)) {
      spdlog::info(
          "Tried associating component {} to entity {} but component wasn't "
          "registered before. Registering component now.",
          component_type_name, entity);
      RegisterComponent<T>();
    }

    GetComponentList<T>()->Insert(entity, component);
    spdlog::debug("Component manager added component {} to entity {}.", component_type_name,
                  entity);
  };

  /// @brief Exposes the component list of type T to the engine so that it can
  /// remove components of type T from entities.
  /// @attention Throws an exception if the component type is not registered.
  /// @param entity The entity from which the component should be removed.
  template <typename T>
  void RemoveComponent(const Entity entity) {
    GetComponentList<T>()->Remove(entity);
    spdlog::debug("Component manager removed component {} from entity {}.", typeid(T).name(),
                  entity);
  };

  /// @brief Exposes the component list of type T to the engine so that it can
  /// retrieve components of type T from entities.
  /// @attention Throws an exception if the component type is not registered.
  /// @param entity The entity from which the component should be retrieved.
  template <typename T>
  T& GetComponent(const Entity entity) {
    spdlog::debug("Component manager retrieves component {} from entity {}.", typeid(T).name(),
                  entity);
    return GetComponentList<T>()->Get(entity);
  };

  /// @brief Returns the component type of type T so that the engine can use it
  /// to create signatures for systems.
  /// @attention Throws an exception if the component type is not registered.
  /// @returns `ComponentType` The component type of type T.
  template <typename T>
  ComponentType GetComponentType() {
    const char* component_type_name = typeid(T).name();

    if (!component_types_.contains(component_type_name)) {
      throw std::runtime_error(
          "Tried to get the component type of a component which is not "
          "registered.");
    }

    ComponentType component_type = component_types_[component_type_name];
    spdlog::debug(
        "Component manager retrieved component type id {} for component of "
        "type {}.",
        component_type, component_type_name);
    return component_type;
  }

  /// @brief This notifies all component lists that an entity was destroyed so
  /// that they can remove the entity from their lists if it exists.
  /// @param entity The entity which was destroyed.
  void EntityWasDestroyed(const Entity entity) {
    for (auto const& pair : component_lists_) {
      auto component_list = pair.second;
      component_list->EntityWasDestroyed(entity);

      spdlog::debug("Component manager removed entity {} from component list {}.", entity,
                    typeid(component_list).name());
    }
    spdlog::debug(
        "Component manager removed entity {} from all component lists it was "
        "associated to.",
        entity);
  }

 private:
  /// maps the actual type to an id
  std::map<const char*, ComponentType> component_types_{};

  /// maps the actual type to a component list
  std::map<const char*, std::shared_ptr<IComponentList>> component_lists_{};

  /// keeps track of the next component type id to use
  ComponentType next_component_type_{};

  /// @brief Provides a shared_ptr instance to the component list of type T so
  /// that it can be interacted with.
  /// @attention Throws an exception if the component type is not registered.
  /// @returns `std::shared_ptr<ComponentList<T>>` The component list of type T.
  template <typename T>
  std::shared_ptr<ComponentList<T>> GetComponentList() {
    const char* component_type_name = typeid(T).name();
    // Create a new instance of shared_ptr so that it isn't moved out of the
    // map. Note: This does not create a new pointer.
    if (!component_lists_.contains(component_type_name)) {
      throw std::runtime_error(
          "Tried to add a component to a component list which does not exist "
          "yet. Try associating the component to an entity first using "
          "AddComponent.");
    }

    return std::static_pointer_cast<ComponentList<T>>(component_lists_[component_type_name]);
  };
};
}  // namespace ecs

#endif  // COMPONENT_MANAGER_H_