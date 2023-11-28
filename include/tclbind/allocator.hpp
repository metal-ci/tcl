//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TCLBIND_ALLOCATOR_HPP
#define TCLBIND_ALLOCATOR_HPP

#include <tcl.h>
#include <memory>

namespace tclbind
{

template<typename T>
struct allocator
{
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;

    allocator() noexcept = default;
    constexpr allocator( const allocator& other ) noexcept = default;
    template< class U >
    constexpr allocator( const allocator<U>& other ) noexcept {}


    [[nodiscard]] constexpr T* allocate( std::size_t n )
    {
        auto p = Tcl_Alloc(n * sizeof(T));
        if (p == nullptr)
            throw std::bad_alloc();
        return reinterpret_cast<T*>(p);
    }

    constexpr void deallocate( T* p, std::size_t n )
    {
        Tcl_Free(reinterpret_cast<char*>(p));
    }
};

}

#endif //TCLBIND_ALLOCATOR_HPP
