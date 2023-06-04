/*
 * The MIT License
 *
 * Copyright 2023 Chistyakov Alexander.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <atomic>

#include <testing/testdefs.h>
#include <testing/utils.h>

#include "intrusive/intrusive_counter.h"
#include "intrusive/intrusive_ptr.h"

namespace {

template<typename T>
class intrusive_fixture : public ::testing::Test
{
public:
    virtual void SetUp() override
    {
        m_mem_usage = ::testing::utils::mem_usage();
    }

    virtual void TearDown() override
    {
        size_t mem_leak = ::testing::utils::mem_usage() - m_mem_usage;
        EXPECT_TRUE(mem_leak == 0) << mem_leak;
    }

private:
    size_t m_mem_usage;
};

class base_counter
{
    INIT_INTRUSIVE_PTR;

public:
    static size_t instance_count;

    base_counter() { ++instance_count; }
    ~base_counter() { --instance_count; }
    size_t use_count() { return m_ref_counter; }
};

size_t base_counter::instance_count = 0;

class child_counter : public base_counter {};

class base_atomic_counter
{
    INIT_ATOMIC_INTRUSIVE_PTR;

public:
    static size_t instance_count;

    base_atomic_counter() { ++instance_count; }
    ~base_atomic_counter() { --instance_count; }
    size_t use_count() { return m_ref_counter; }
};

size_t base_atomic_counter::instance_count = 0;
/*
class child_atomic_counter : public base_atomic_counter {};
*/
struct counter
{
    using base = base_counter;
    using child = child_counter;
};

struct atomic_counter
{
    using base = base_atomic_counter;
    using child = base_atomic_counter;
};

using types = testing::Types<counter, atomic_counter>;
TYPED_TEST_SUITE(intrusive_fixture, types);

} // <anonymous> namespace

TYPED_TEST(intrusive_fixture, constructor_default)
{
    using base = typename TypeParam::base;

    ::wstux::intrusive_ptr<base> ptr;
    EXPECT_TRUE(ptr.get() == NULL);
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, constructor_pointer_null)
{
    using base = typename TypeParam::base;

    ::wstux::intrusive_ptr<base> ptr(NULL);
    EXPECT_TRUE(ptr.get() == NULL);
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, constructor_pointer)
{
    using base = typename TypeParam::base;

    base* p_base = new base;
    EXPECT_TRUE(p_base->use_count() == 0);
    EXPECT_TRUE(base::instance_count == 1);

    {
        ::wstux::intrusive_ptr<base> ptr(p_base);
        EXPECT_TRUE(ptr.get() == p_base);
        EXPECT_TRUE(p_base->use_count() == 1);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, constructor_pointer_in_place)
{
    using base = typename TypeParam::base;

    EXPECT_TRUE(base::instance_count == 0);
    {
        ::wstux::intrusive_ptr<base> ptr(new base);
        ASSERT_TRUE(ptr.get() != NULL);
        EXPECT_TRUE(ptr->use_count() == 1);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, constructor_copy)
{
    using base = typename TypeParam::base;

    base* p_base = new base;
    EXPECT_TRUE(p_base->use_count() == 0);
    EXPECT_TRUE(base::instance_count == 1);

    {
        ::wstux::intrusive_ptr<base> ptr_1(p_base);
        ::wstux::intrusive_ptr<base> ptr_2(ptr_1);
        EXPECT_TRUE(ptr_1.get() == ptr_2.get());
        EXPECT_TRUE(ptr_1->use_count() == 2);
        EXPECT_TRUE(ptr_2->use_count() == 2);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, constructor_copy_in_place)
{
    using base = typename TypeParam::base;

    EXPECT_TRUE(base::instance_count == 0);
    {
        ::wstux::intrusive_ptr<base> ptr_1(new base);
        ::wstux::intrusive_ptr<base> ptr_2(ptr_1);
        EXPECT_TRUE(ptr_1.get() == ptr_2.get());
        EXPECT_TRUE(ptr_1->use_count() == 2);
        EXPECT_TRUE(ptr_2->use_count() == 2);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, constructor_copy_null_1)
{
    using base = typename TypeParam::base;

    ::wstux::intrusive_ptr<base> ptr_1;
    ::wstux::intrusive_ptr<base> ptr_2(ptr_1);
    ASSERT_TRUE(ptr_1.get() == NULL);
    ASSERT_TRUE(ptr_2.get() == NULL);
    EXPECT_TRUE(ptr_1.get() == ptr_2.get());
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, constructor_copy_null_2)
{
    using base = typename TypeParam::base;

    ::wstux::intrusive_ptr<base> ptr_1(NULL);
    ::wstux::intrusive_ptr<base> ptr_2(ptr_1);
    EXPECT_TRUE(ptr_1.get() == ptr_2.get());
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, constructor_copy_inheritance)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    child* p_child = new child;
    EXPECT_TRUE(p_child->use_count() == 0);
    EXPECT_TRUE(base::instance_count == 1);

    {
        ::wstux::intrusive_ptr<child> ptr_1(p_child);
        ::wstux::intrusive_ptr<base> ptr_2(ptr_1);
        EXPECT_TRUE(ptr_1.get() == ptr_2.get());
        EXPECT_TRUE(ptr_1->use_count() == 2);
        EXPECT_TRUE(ptr_2->use_count() == 2);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, constructor_copy_inheritance_in_place_1)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    EXPECT_TRUE(base::instance_count == 0);
    {
        ::wstux::intrusive_ptr<child> ptr_1(new child);
        ::wstux::intrusive_ptr<base> ptr_2(ptr_1);
        EXPECT_TRUE(ptr_1.get() == ptr_2.get());
        EXPECT_TRUE(ptr_1->use_count() == 2);
        EXPECT_TRUE(ptr_2->use_count() == 2);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, constructor_copy_inheritance_in_place_2)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    EXPECT_TRUE(base::instance_count == 0);
    {
        ::wstux::intrusive_ptr<base> ptr_1(new child);
        ::wstux::intrusive_ptr<base> ptr_2(ptr_1);
        EXPECT_TRUE(ptr_1.get() == ptr_2.get());
        EXPECT_TRUE(ptr_1->use_count() == 2);
        EXPECT_TRUE(ptr_2->use_count() == 2);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, constructor_copy_inheritance_null_1)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    ::wstux::intrusive_ptr<child> ptr_1;
    ::wstux::intrusive_ptr<base> ptr_2(ptr_1);
    EXPECT_TRUE(ptr_1.get() == ptr_2.get());
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, constructor_copy_inheritance_null_2)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    ::wstux::intrusive_ptr<child> ptr_1(NULL);
    ::wstux::intrusive_ptr<base> ptr_2(ptr_1);
    EXPECT_TRUE(ptr_1.get() == ptr_2.get());
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, destructor_1)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    EXPECT_TRUE(base::instance_count == 0);
    {
        ::wstux::intrusive_ptr<base> ptr_1(new child);
        EXPECT_TRUE(ptr_1->use_count() == 1);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, destructor_2)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    EXPECT_TRUE(base::instance_count == 0);
    {
        ::wstux::intrusive_ptr<base> ptr_1(new child);
        EXPECT_TRUE(ptr_1->use_count() == 1);
        EXPECT_TRUE(base::instance_count == 1);
        {
            ::wstux::intrusive_ptr<base> ptr_2(ptr_1);
            EXPECT_TRUE(ptr_1.get() == ptr_2.get());
            EXPECT_TRUE(ptr_1->use_count() == 2);
            EXPECT_TRUE(ptr_2->use_count() == 2);
            EXPECT_TRUE(base::instance_count == 1);
        }
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, destructor_3)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    EXPECT_TRUE(base::instance_count == 0);
    ::wstux::intrusive_ptr<base> ptr_1(new child);
    EXPECT_TRUE(ptr_1->use_count() == 1);
    EXPECT_TRUE(base::instance_count == 1);
    {
        ::wstux::intrusive_ptr<base> ptr_2(ptr_1);
        EXPECT_TRUE(ptr_1.get() == ptr_2.get());
        EXPECT_TRUE(ptr_1->use_count() == 2);
        EXPECT_TRUE(ptr_2->use_count() == 2);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 1);
}

TYPED_TEST(intrusive_fixture, operator_copy)
{
    using base = typename TypeParam::base;

    base* p_base = new base;
    EXPECT_TRUE(p_base->use_count() == 0);
    EXPECT_TRUE(base::instance_count == 1);

    {
        ::wstux::intrusive_ptr<base> ptr_1(p_base);
        ::wstux::intrusive_ptr<base> ptr_2;
        ptr_2 = ptr_1;
        EXPECT_TRUE(ptr_1.get() == ptr_2.get());
        EXPECT_TRUE(ptr_1->use_count() == 2);
        EXPECT_TRUE(ptr_2->use_count() == 2);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, operator_copy_in_place)
{
    using base = typename TypeParam::base;

    EXPECT_TRUE(base::instance_count == 0);
    {
        ::wstux::intrusive_ptr<base> ptr_1(new base);
        ::wstux::intrusive_ptr<base> ptr_2;
        ptr_2 = ptr_1;
        EXPECT_TRUE(ptr_1.get() == ptr_2.get());
        EXPECT_TRUE(ptr_1->use_count() == 2);
        EXPECT_TRUE(ptr_2->use_count() == 2);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, operator_copy_null_1)
{
    using base = typename TypeParam::base;

    ::wstux::intrusive_ptr<base> ptr_1;
    ::wstux::intrusive_ptr<base> ptr_2;
    ptr_2 = ptr_1;
    ASSERT_TRUE(ptr_1.get() == NULL);
    ASSERT_TRUE(ptr_2.get() == NULL);
    EXPECT_TRUE(ptr_1.get() == ptr_2.get());
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, operator_copy_null_2)
{
    using base = typename TypeParam::base;

    ::wstux::intrusive_ptr<base> ptr_1(NULL);
    ::wstux::intrusive_ptr<base> ptr_2;
    ptr_2 = ptr_1;
    EXPECT_TRUE(ptr_1.get() == ptr_2.get());
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, operator_copy_inheritance)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    child* p_child = new child;
    EXPECT_TRUE(p_child->use_count() == 0);
    EXPECT_TRUE(base::instance_count == 1);

    {
        ::wstux::intrusive_ptr<child> ptr_1(p_child);
        ::wstux::intrusive_ptr<base> ptr_2;
        ptr_2 = ptr_1;
        EXPECT_TRUE(ptr_1.get() == ptr_2.get());
        EXPECT_TRUE(ptr_1->use_count() == 2);
        EXPECT_TRUE(ptr_2->use_count() == 2);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, operator_copy_inheritance_in_place_1)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    EXPECT_TRUE(base::instance_count == 0);
    {
        ::wstux::intrusive_ptr<child> ptr_1(new child);
        ::wstux::intrusive_ptr<base> ptr_2;
        ptr_2 = ptr_1;
        EXPECT_TRUE(ptr_1.get() == ptr_2.get());
        EXPECT_TRUE(ptr_1->use_count() == 2);
        EXPECT_TRUE(ptr_2->use_count() == 2);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, operator_copy_inheritance_in_place_2)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    EXPECT_TRUE(base::instance_count == 0);
    {
        ::wstux::intrusive_ptr<base> ptr_1(new child);
        ::wstux::intrusive_ptr<base> ptr_2;
        ptr_2 = ptr_1;
        EXPECT_TRUE(ptr_1.get() == ptr_2.get());
        EXPECT_TRUE(ptr_1->use_count() == 2);
        EXPECT_TRUE(ptr_2->use_count() == 2);
        EXPECT_TRUE(base::instance_count == 1);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, operator_copy_inheritance_in_place_3)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    EXPECT_TRUE(base::instance_count == 0);
    {
        ::wstux::intrusive_ptr<base> ptr_1(new child);
        ::wstux::intrusive_ptr<base> ptr_2;
        EXPECT_TRUE(base::instance_count == 1);
        ptr_1 = ptr_2;
        EXPECT_TRUE(ptr_1.get() == NULL);
        EXPECT_TRUE(ptr_1.get() == ptr_2.get());
        EXPECT_TRUE(base::instance_count == 0);
    }
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, operator_copy_inheritance_null_1)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    ::wstux::intrusive_ptr<child> ptr_1;
    ::wstux::intrusive_ptr<base> ptr_2;
    ptr_2 = ptr_1;
    EXPECT_TRUE(ptr_1.get() == ptr_2.get());
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, operator_copy_inheritance_null_2)
{
    using base = typename TypeParam::base;
    using child = typename TypeParam::child;

    ::wstux::intrusive_ptr<child> ptr_1(NULL);
    ::wstux::intrusive_ptr<base> ptr_2;
    ptr_2 = ptr_1;
    EXPECT_TRUE(ptr_1.get() == ptr_2.get());
    EXPECT_TRUE(base::instance_count == 0);
}

TYPED_TEST(intrusive_fixture, operator_comparison_1)
{
    using base = typename TypeParam::base;

    EXPECT_TRUE(base::instance_count == 0);
    
    ::wstux::intrusive_ptr<base> ptr;
    
    EXPECT_TRUE(ptr == ptr);
    EXPECT_FALSE(ptr);
    EXPECT_TRUE(! ptr);
}

TYPED_TEST(intrusive_fixture, operator_comparison_2)
{
    using base = typename TypeParam::base;

    EXPECT_TRUE(base::instance_count == 0);
    
    ::wstux::intrusive_ptr<base> ptr_1;
    ::wstux::intrusive_ptr<base> ptr_2;
    
    EXPECT_TRUE(ptr_1 == ptr_2);
}

TYPED_TEST(intrusive_fixture, operator_comparison_3)
{
    using base = typename TypeParam::base;

    EXPECT_TRUE(base::instance_count == 0);
    
    ::wstux::intrusive_ptr<base> ptr_1;
    ::wstux::intrusive_ptr<base> ptr_2(ptr_1);
    
    EXPECT_TRUE(ptr_1 == ptr_2);
    EXPECT_FALSE(ptr_1);
    EXPECT_TRUE(! ptr_1);
}

int main(int /*argc*/, char** /*argv*/)
{
    return RUN_ALL_TESTS();
}
