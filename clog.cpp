#include "clog.h"

#ifdef NDEBUG
bool my::clog::debug_ = false;
#else
bool my::clog::debug_ = true;
#endif

