#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"

#define TEST(c, ...) \
	( (c) || (error(#c " failed: " __VA_ARGS__),0) )

int main(void)
{
	char buf[100];
	struct timespec ts;
	sem_t *sem, *sem2;
	int val;

	clock_gettime(CLOCK_REALTIME, &ts);
	snprintf(buf, sizeof buf, "/testsuite-%d-%d", (int)getpid(), (int)ts.tv_nsec);

	TEST((sem=sem_open(buf, O_CREAT|O_EXCL, 0700, 1)) != SEM_FAILED,
		"could not open sem: %s\n", strerror(errno));
	errno = 0;
	TEST(sem_open(buf, O_CREAT|O_EXCL, 0700, 1) == SEM_FAILED,
		"reopening should fail with O_EXCL\n");
	TEST(errno == EEXIST,
		"after reopen failure errno is \"%s\" (%d); want EEXIST (%d)\n", strerror(errno), errno, EEXIST);
	errno = 0;

	TEST(sem_getvalue(sem, &val) == 0, "failed to get sem value\n");
	TEST(val == 1, "wrong initial semaphore value: %d\n", val);

	TEST((sem2=sem_open(buf, 0)) == sem,
		"could not reopen sem: got %p, want %p\n", sem2, sem);

	TEST(sem_wait(sem) == 0, "%s\n", strerror(errno));
	TEST(sem_getvalue(sem2, &val) == 0, "%s\n", strerror(errno));
	TEST(val == 0, "wrong semaphore value on second handle: %d\n", val);

	TEST(sem_post(sem) == 0, "%s\n", strerror(errno));
	TEST(sem_getvalue(sem2, &val) == 0, "%s\n", strerror(errno));
	TEST(val == 1, "wrong semaphore value on second handle: %d\n", val);

	TEST(sem_close(sem) == 0, "%s\n", strerror(errno));
	TEST(sem_close(sem) == 0, "%s\n", strerror(errno));
	TEST(sem_unlink(buf) == 0, "%s\n", strerror(errno));
	return test_status;
}
