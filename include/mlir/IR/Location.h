//===- Location.h - MLIR Location Classes -----------------------*- C++ -*-===//
//
// Copyright 2019 The MLIR Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================
//
// These classes provide the ability to relate MLIR objects back to source
// location position information.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_IR_LOCATION_H
#define MLIR_IR_LOCATION_H

#include "mlir/Support/LLVM.h"
#include "llvm/ADT/DenseMapInfo.h"

namespace mlir {

class Attribute;
class MLIRContext;
class Identifier;

namespace detail {

struct LocationStorage;
struct UnknownLocationStorage;
struct FileLineColLocationStorage;
struct NameLocationStorage;
struct CallSiteLocationStorage;
struct FusedLocationStorage;

} // namespace detail

/// Location objects represent source locations information in MLIR.
class Location {
public:
  enum class Kind : uint8_t {
    /// This represents an unknown location.
    UnknownLocation,

    /// This represents a file/line/column location.
    FileLineColLocation,

    /// This represents an identity name attached to a child location.
    NameLocation,

    /// This represents a location as a call site.
    CallSiteLocation,

    // Represents a location as a 'void*' pointer to a front-end's opaque
    // location information, which must live longer than the MLIR objects that
    // refer to it.  OpaqueLocation's are never serialized.
    //
    // TODO: OpaqueLocation,

    // Represents a value inlined through a function call.
    // TODO: InlinedLocation,

    // Represents a value composed of multiple source constructs.
    FusedLocation,
  };

  using ImplType = detail::LocationStorage;

  /* implicit */ Location(const ImplType *loc)
      : loc(const_cast<ImplType *>(loc)) {
    assert(loc && "location should never be null.");
  }

  Location() = delete;
  Location(const Location &other) : loc(other.loc) {}
  Location &operator=(Location other) {
    loc = other.loc;
    return *this;
  }

  bool operator==(Location other) const { return loc == other.loc; }
  bool operator!=(Location other) const { return !(*this == other); }

  template <typename U> bool isa() const;
  template <typename U> Optional<U> dyn_cast() const;
  template <typename U> U cast() const;

  /// Return the classification for this location.
  Kind getKind() const;

  /// Print the location.
  void print(raw_ostream &os) const;
  void dump() const;

  friend ::llvm::hash_code hash_value(Location arg);

  /// Methods for supporting PointerLikeTypeTraits.
  const void *getAsOpaquePointer() const {
    return static_cast<const void *>(loc);
  }
  static Location getFromOpaquePointer(const void *pointer) {
    return Location(reinterpret_cast<ImplType *>(const_cast<void *>(pointer)));
  }

protected:
  ImplType *loc;
};

inline raw_ostream &operator<<(raw_ostream &os, const Location &loc) {
  loc.print(os);
  return os;
}

/// Represents an unknown location.  This is always a singleton for a given
/// MLIRContext.
class UnknownLoc : public Location {
public:
  using ImplType = detail::UnknownLocationStorage;
  using Location::Location;

  static UnknownLoc get(MLIRContext *context);

  /// Methods for support type inquiry through isa, cast, and dyn_cast.
  static bool kindof(Kind kind) { return kind == Kind::UnknownLocation; }
};

/// Represents a location derived from a file/line/column location.  The column
/// and line may be zero to represent unknown column and/or unknown line/column
/// information.
class FileLineColLoc : public Location {
public:
  using ImplType = detail::FileLineColLocationStorage;
  using Location::Location;

  /// Return a uniqued FileLineCol location object.
  static FileLineColLoc get(Identifier filename, unsigned line, unsigned column,
                            MLIRContext *context);
  static FileLineColLoc get(StringRef filename, unsigned line, unsigned column,
                            MLIRContext *context);

  StringRef getFilename() const;

  unsigned getLine() const;
  unsigned getColumn() const;

  /// Methods for support type inquiry through isa, cast, and dyn_cast.
  static bool kindof(Kind kind) { return kind == Kind::FileLineColLocation; }
};

/// Represents an identity name attached to a child location.
class NameLoc : public Location {
public:
  using ImplType = detail::NameLocationStorage;
  using Location::Location;

  /// Return a uniqued name location object. The child location must not be
  /// another NameLoc.
  static NameLoc get(Identifier name, Location child, MLIRContext *context);

  /// Return a uniqued name location object with an unknown child.
  static NameLoc get(Identifier name, MLIRContext *context);

  /// Return the name identifier.
  Identifier getName() const;

  /// Return the child location.
  Location getChildLoc() const;

  /// Methods for support type inquiry through isa, cast, and dyn_cast.
  static bool kindof(Kind kind) { return kind == Kind::NameLocation; }
};

/// Represents a location as call site. "callee" is the concrete location
/// (Unknown/NameLocation/FileLineColLoc) and "caller" points to the caller's
/// location (another CallLocation or a concrete location). Multiple
/// CallSiteLocs can be chained to form a call stack.
class CallSiteLoc : public Location {
public:
  using ImplType = detail::CallSiteLocationStorage;
  using Location::Location;

  /// Return a uniqued call location object.
  static CallSiteLoc get(Location callee, Location caller,
                         MLIRContext *context);

  /// Return a call site location which represents a name reference in one line
  /// or a stack of frames. The input frames are ordered from innermost to
  /// outermost.
  static CallSiteLoc get(Location name, ArrayRef<Location> frames,
                         MLIRContext *context);

  /// The concrete location information this object presents.
  Location getCallee() const;

  /// The caller's location.
  Location getCaller() const;

  /// Methods for support type inquiry through isa, cast, and dyn_cast.
  static bool kindof(Kind kind) { return kind == Kind::CallSiteLocation; }
};

/// Represents a value composed of multiple source constructs, with an optional
/// metadata attribute.
class FusedLoc : public Location {
public:
  using ImplType = detail::FusedLocationStorage;
  using Location::Location;

  /// Return a uniqued Fused Location object. The first location in the list
  /// will get precedence during diagnostic emission, with the rest being
  /// displayed as supplementary "fused from here" style notes.
  static Location get(ArrayRef<Location> locs, MLIRContext *context);
  static Location get(ArrayRef<Location> locs, Attribute metadata,
                      MLIRContext *context);

  ArrayRef<Location> getLocations() const;

  /// Returns the optional metadata attached to this fused location. Given that
  /// it is optional, the return value may be a null node.
  Attribute getMetadata() const;

  /// Methods for support type inquiry through isa, cast, and dyn_cast.
  static bool kindof(Kind kind) { return kind == Kind::FusedLocation; }
};

// Make Location hashable.
inline ::llvm::hash_code hash_value(Location arg) {
  return ::llvm::hash_value(arg.loc);
}

template <typename U> bool Location::isa() const {
  return U::kindof(getKind());
}
template <typename U> Optional<U> Location::dyn_cast() const {
  return isa<U>() ? U(loc) : Optional<U>();
}
template <typename U> U Location::cast() const {
  assert(isa<U>());
  return U(loc);
}

} // end namespace mlir

namespace llvm {

// Type hash just like pointers.
template <> struct DenseMapInfo<mlir::Location> {
  static mlir::Location getEmptyKey() {
    auto pointer = llvm::DenseMapInfo<void *>::getEmptyKey();
    return mlir::Location(static_cast<mlir::Location::ImplType *>(pointer));
  }
  static mlir::Location getTombstoneKey() {
    auto pointer = llvm::DenseMapInfo<void *>::getTombstoneKey();
    return mlir::Location(static_cast<mlir::Location::ImplType *>(pointer));
  }
  static unsigned getHashValue(mlir::Location val) {
    return mlir::hash_value(val);
  }
  static bool isEqual(mlir::Location LHS, mlir::Location RHS) {
    return LHS == RHS;
  }
};

/// We align LocationStorage by 8, so allow LLVM to steal the low bits.
template <> struct PointerLikeTypeTraits<mlir::Location> {
public:
  static inline void *getAsVoidPointer(mlir::Location I) {
    return const_cast<void *>(I.getAsOpaquePointer());
  }
  static inline mlir::Location getFromVoidPointer(void *P) {
    return mlir::Location::getFromOpaquePointer(P);
  }
  enum { NumLowBitsAvailable = 3 };
};

} // namespace llvm

#endif
