#include <sys/types.h>
#include <event.h>
#include <stdio.h>

int main()
{
	unsigned int version;
#if defined(HAVE_EVENT_GET_VERSION_NUMBER)
	version = event_get_version_number();
#else
	unsigned int major, minor, patchlevel;
	char c;
	int fields;
	fields = sscanf(event_get_version(), "%u.%u.%u%c", &major, &minor, &patchlevel, &c);
	version = (((major) << 24) | ((minor) << 16) | ((patchlevel) << 8));
#endif
	return (version < EVENT_VERSION_WANTED);
}
