#ifdef INTERFACE_DEFINITION
    #define PURE = 0
#else
    #define PURE
#endif

    virtual void retranslateUi() PURE;

#undef PURE
