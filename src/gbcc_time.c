#include "gbcc_time.h"

uint64_t gbcc_time_diff(const struct timespec * const cur,
		const struct timespec * const old)
{
	uint64_t sec = (uint64_t)(cur->tv_sec - old->tv_sec);
	uint64_t nsec;
	if (sec == 0) {
		nsec = (uint64_t)(cur->tv_nsec - old->tv_nsec);
		return nsec;
	}
	nsec = SECOND * sec;
	if (old->tv_nsec > cur->tv_nsec) {
		nsec -= (uint64_t)(old->tv_nsec - cur->tv_nsec);
	} else {
		nsec += (uint64_t)(cur->tv_nsec - old->tv_nsec);
	}
	return nsec;
}
