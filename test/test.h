#ifndef _TEST_H_
#define _TEST_H_

#include <psock.h>

#if 1
#define AF_TEST AF_PSOCK
#else
#define AF_TEST AF_INET
#endif

#endif /* _TEST_H_ */
