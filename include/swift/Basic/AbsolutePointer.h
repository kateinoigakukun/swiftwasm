//===--- AbsolutePointer.h - Relative Pointer Support -----------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2019 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//


#ifndef SWIFT_BASIC_ABSOLUTEPOINTER_H
#define SWIFT_BASIC_ABSOLUTEPOINTER_H

namespace swift {

template<typename ValueTy, bool Nullable = false, typename Size>
class AbsolutePointer {
private:
    static_assert(std::is_integral<Size>::value &&
                  std::is_unsigned<Size>::value,
                  "Size type should be unsigned integer");
    
    Size Ptr;
public:
    const ValueTy *get() const & {
        return *reinterpret_cast<const ValueTy * const>(Ptr);
    }
}
}

#endif // SWIFT_BASIC_ABSOLUTEPOINTER_H
