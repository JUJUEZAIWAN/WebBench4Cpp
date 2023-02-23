

#ifndef NOCOPY_H
#define NOCOPY_H

#define DISABLE_COPY(CLASS_NAME)                        \
    CLASS_NAME(const CLASS_NAME &) = delete;            \
    CLASS_NAME(CLASS_NAME &&) = delete;                 \
    CLASS_NAME &operator=(const CLASS_NAME &) = delete; \
    CLASS_NAME &operator=(CLASS_NAME &&) = delete;

#endif