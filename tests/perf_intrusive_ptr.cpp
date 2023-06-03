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
#include <memory>

#include <testing/perfdefs.h>
#include <testing/utils.h>

#include "intrusive/intrusive_ptr.h"

namespace {

static const size_t kIterationCount = 1000000;

template<typename T>
class intrusive_fixture : public ::testing::Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};


class base_counter
{
    INIT_INTRUSIVE_PTR;
public:
    size_t counter = 0;
};

class base_atomic_counter
{
    INIT_ATOMIC_INTRUSIVE_PTR;
public:
    size_t counter = 0;
};

using types = testing::Types<::wstux::intrusive_ptr<base_counter>,
                             ::wstux::intrusive_ptr<base_atomic_counter>,
                             std::shared_ptr<base_counter>>;
TYPED_PERF_TEST_SUITE(intrusive_fixture, types);

}

TYPED_PERF_TEST(intrusive_fixture, create_new)
{
    PERF_INIT_TIMER(create_new_perf);

    using smart_ptr = TypeParam;
    using element_type = typename smart_ptr::element_type;

    size_t dummy = 0;
    const double begin = ::testing::utils::cpu_time_msecs_self();
    PERF_START_TIMER(create_new_perf);
    for (size_t i = 0; i < kIterationCount; ++i) {
        smart_ptr ptr(new element_type());
        dummy += ++ptr->counter;
    }
    PERF_PAUSE_TIMER(create_new_perf);
    const double end = ::testing::utils::cpu_time_msecs_self();
    PERF_MESSAGE() << "iteration count: " << dummy << "; "
                   << "cpu time: " << (end - begin) << " msecs";
}

int main(int /*argc*/, char** /*argv*/)
{
    return RUN_ALL_PERF_TESTS();
}

