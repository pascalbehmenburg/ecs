#ifndef ENTITY_MANAGER_H_
#define ENTITY_MANAGER_H_

#include <fmt/format.h>

#include <array>
#include <cstdint>
#include <memory>
#include <queue>

#include "type_def.h"

namespace ecs {
/// @file
/// @brief The entity manager manages all entities that ever exist within the
/// ECS and exposes methods for the Engine to interact on them.
/// @details Using a queue which keeps track of the entities that are available
/// and a signature array which keeps track of the signatures of the entities.
/// It exposes functions to interact with those data structures. It's purposed
/// to be coordinated by the engine.
class EntityManager final {
 public:
  /// @brief Creates an entity manager and pre-allocates a queue with all
  /// possible entity IDs to prohibit memory allocations later in the program.
  EntityManager() {
    for (Entity entity = 0; entity < kMaxEntities; ++entity) {
      available_entities_.push(entity);
    }
    spdlog::debug("Entity manager pre-allocated {} entities.", kMaxEntities);
  }

  /// @brief Creates an entity and returns its ID so that one is able to assign
  /// components to it and therefore give the entity a meaning.
  /// @attention Throws an exception if the maximum number of entities is
  /// exceeded.
  /// @returns `Entity` The ID of the created entity.
  Entity CreateEntity() {
    if (available_entities_.empty()) {
      throw std::runtime_error(
          fmt::format("Maximum number of living entities {} exceeded, you may "
                      "want to increase the kMaxEntities.",
                      kMaxEntities));
    }

    Entity entity = available_entities_.front();
    available_entities_.pop();
    spdlog::debug("Entity manager created entity {}.", entity);
    return entity;
  }

  /// @brief Destroys an entity and makes its ID available through the queue
  /// again so that it can be reused for a new entity.
  /// @attention Throws an exception if the entity is out of bound.
  /// @param entity The entity which should be destroyed.
  void DestroyEntity(const Entity entity) {
    CheckEntityOutOfBound(entity);
    entity_signatures_[entity].reset();
    available_entities_.push(entity);
    spdlog::debug("Entity manager deleted entity {}.", entity);
  }

  /// @brief Sets the signature of an entity so that it can be used for
  /// compatibility checks.
  /// @attention Throws an exception if the entity is out of bound.
  /// @param entity The entity whose signature should be set.
  /// @param signature The signature which should be set.
  void SetSignature(const Entity entity, const Signature signature) {
    CheckEntityOutOfBound(entity);
    entity_signatures_[entity] = signature;
    spdlog::debug("Entity manager persisted signature {} for entity {}.", signature.to_string(),
                  entity);
  }

  /// @brief Returns the signature of an entity.
  /// @attention Throws an exception if the entity is out of bound.
  /// @param entity The entity whose signature should be returned.
  /// @returns `Signature` The signature of the entity.
  Signature GetSignature(const Entity entity) {
    CheckEntityOutOfBound(entity);
    Signature signature = entity_signatures_[entity];
    spdlog::debug("Entity manager retrieved signature {} for entity {}.", signature.to_string(),
                  entity);
    return signature;
  }

 private:
  /// contains all entities that will ever be available
  std::queue<Entity> available_entities_;

  /// contains the signatures of all entities
  std::array<Signature, kMaxEntities> entity_signatures_;

  /// @brief Checks if an entity is out of bound so that we can ensure we never
  /// have more entities than allocated at the instantiation of the entity
  /// manager.
  /// @attention Throws an exception if it is out of bound.
  /// @param entity The entity which should be checked.
  static void CheckEntityOutOfBound(Entity entity) {
    if (entity >= kMaxEntities) {
      throw std::runtime_error(fmt::format(
          "The entity {} exceeds the maximum number of entities {}.", entity, kMaxEntities));
    }
  }
};
}  // namespace ecs

#endif  // ENTITY_MANAGER_H_