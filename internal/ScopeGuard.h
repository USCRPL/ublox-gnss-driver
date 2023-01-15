#ifndef SCOPE_GUARD_H
#define SCOPE_GUARD_H

template <typename InitFunction, typename CleanupFunction> class ScopeGuard
{
public:
    ScopeGuard(InitFunction& init, CleanupFunction& cleanup)
        : init_(init)
        , cleanup_(cleanup)
        , engaged_(true)
    {
        init();
    }

    ~ScopeGuard()
    {
        if (engaged_)
        {
            cleanup();
        }
    }

    void invalidate()
    {
        engaged_ = false;
        /* Resources will no longer be released */
    }

    void cleanup()
    {
        cleanup_();
        invalidate();
    }

private:
    InitFunction& init_;
    CleanupFunction& cleanup_;
    bool engaged_;
};

#endif
