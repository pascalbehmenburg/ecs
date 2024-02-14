#ifndef TYPE_DEF_H_
#define TYPE_DEF_H_

#include <bitset>
#include <cstdint>

namespace ecs {
/// caps the number of components that can be created this should stay 64 for
/// now since one may want to exchange the std::bitset with a custom
/// implementation that just wraps an uint64_t
constexpr uint8_t kMaxComponents = 64;

/// caps the number of entities that can be created to enable pre-allocation of
/// a queue of entities in the EntityManager
constexpr uint16_t kMaxEntities = 65535;

/// An entity is just an ID which is used to identify an entity within the ECS.
using Entity = uint32_t;

/// A signature is a bitset which is used to identify the components of an
/// entity or system. The bitset is also used to enable fast bitwise operations
/// to check compatibility of entities with systems.
using Signature = std::bitset<kMaxComponents>;

/// A component type is a unique ID which maps a component to a specific bit of
/// the signatures. For example ComponentType 0 maps to the first bit of the
/// signature, ComponentType 1 maps to the second bit of the signature and so
/// on.
using ComponentType = uint16_t;
}  // namespace ecs

#endif  // TYPE_DEF_H_