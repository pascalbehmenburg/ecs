#ifndef SYSTEM_MANAGER_H_
#define SYSTEM_MANAGER_H_

#include <spdlog/spdlog.h>

#include <map>
#include <memory>
#include <stdexcept>

#include "system.h"
#include "type_def.h"

namespace ecs {
/// @file
/// @brief The system manager manages all systems that ever exist within the
/// ECS and exposes methods for the Engine to interact on them.
/// @details It uses maps to keep track of the systems and their signatures.
class SystemManager final {
 public:
  /// @brief Registers a system (type) which was created using a non-default
  /// constructor before.
  /// @attention Throws an exception if the system type is already registered.
  /// @param system The system which was already constructed and should be
  /// registered.
  /// @returns `std::shared_ptr<T>` The system of type T.
  template <typename T>
  std::shared_ptr<T> RegisterSystem(T system) {
    const char* system_type_name = typeid(T).name();

    if (systems_.contains(system_type_name)) {
      throw std::runtime_error(fmt::format(
          "Tried to register a system {} which is already registered.", system_type_name));
    }

    std::shared_ptr<T> system_ptr = std::make_shared<T>(system);
    systems_[system_type_name] = system_ptr;

    spdlog::debug("System manager registered system {}.", system_type_name);

    return system_ptr;
  }

  /// @brief Registers a system type so that every system will only exist once
  /// and can be accessed to assign a signature to it.
  /// @attention Throws an exception if the system type is already registered.
  /// @returns `std::shared_ptr<T>` The system of type T.
  template <typename T>
  std::shared_ptr<T> RegisterSystem() {
    const char* system_type_name = typeid(T).name();

    if (systems_.contains(system_type_name)) {
      throw std::runtime_error(fmt::format(
          "Tried to register a system {} which is already registered.", system_type_name));
    }

    std::shared_ptr<T> system = std::make_shared<T>();
    systems_[system_type_name] = system;

    spdlog::debug("System manager registered system {}.", system_type_name);

    return system;
  };

  /// @brief Sets the signature of a system type so that entities can be
  /// associated to it.
  /// @attention Throws an exception if the system type is not registered.
  /// @param signature The signature which should be set for the system type.
  template <typename T>
  void SetSystemSignature(const Signature signature) {
    const char* system_type_name = typeid(T).name();

    if (!systems_.contains(system_type_name)) {
      throw std::runtime_error(
          fmt::format("Tried to set the signature of system type {} which is not "
                      "registered",
                      system_type_name));
    }

    system_signatures_[system_type_name] = signature;
    spdlog::debug("System manager set signature {} to system {}.", signature.to_string(),
                  system_type_name);
  };

  /// @brief Returns the system of type T to be used in other system.
  /// @attention Throws an exception if the system type is not registered.
  /// @returns `std::shared_ptr<T>` The system of type T.
  template <typename T>
  std::shared_ptr<T> GetSystem() {
    const char* system_type_name = typeid(T).name();

    if (!systems_.contains(system_type_name)) {
      throw std::runtime_error(fmt::format(
          "Tried to retrieve system {} which is not yet registered.", system_type_name));
    }

    spdlog::debug("System manager retrieves system {}.", system_type_name);
    return std::dynamic_pointer_cast<T>(systems_[system_type_name]);
  }

  /// @brief Is used to notify the system manager that an entcity was destroyed
  /// so that the system manager can remove the entity from all systems known to
  /// him.
  /// @param entity The entity which was destroyed.
  void EntityWasDestroyed(const Entity entity) {
    for (auto const& pair : systems_) {
      auto const& system = pair.second;
      system->entities_.erase(entity);
      spdlog::debug("System manager erased entity {} from system {}.", entity,
                    typeid(system).name());
    }

    spdlog::debug(
        "System manager erased entity {} from all systems it was associated "
        "to.",
        entity);
  };

  /// @brief Is used to notify the system manager that the signature of an
  /// entity changed so that the system manager can add or remove the entity
  /// from all systems known to it.
  /// @param entity The entity whose signature changed.
  /// @param entity_signature The new signature of the entity.
  void EntitySignatureChanged(const Entity entity, const Signature entity_signature) {
    for (auto const& pair : systems_) {
      auto const& type = pair.first;
      auto const& system = pair.second;
      auto const& system_signature = system_signatures_[type];

      // check if the entity has all the components required by the system
      // note: 0111 & 0011 = 0011
      auto const compatibility_sig = (entity_signature & system_signature);
      if (compatibility_sig == system_signature) {
        spdlog::debug("System manager found entity {} compatible with system {}.", entity,
                      typeid(system).name());

        system->entities_.insert(entity);

        spdlog::debug(
            "System manager added entity {} to system {} because of updated "
            "signature.",
            entity, typeid(system).name());
      } else {
        system->entities_.erase(entity);

        spdlog::debug(
            "System manager removed entity {} from system {} because of "
            "updated signature.",
            entity, typeid(system).name());
      }
    }

    spdlog::debug("System manager updated all systems based on entity signature change.");
  };

 private:
  /// maps system type names to actual systems
  std::map<const char*, std::shared_ptr<System>> systems_;

  /// maps system type names to signatures
  std::map<const char*, Signature> system_signatures_;
};
}  // namespace ecs

#endif  // SYSTEM_MANAGER_H_