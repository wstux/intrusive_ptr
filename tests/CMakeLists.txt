TestTarget(ut_intrusive_ptr
    SOURCES
        ut_intrusive_ptr.cpp
    LIBRARIES
        intrusive
    DEPENDS
        testing
)

ExecTarget(perf_intrusive_ptr
    SOURCES
        perf_intrusive_ptr.cpp
    LIBRARIES
        intrusive
    DEPENDS
        testing
)

