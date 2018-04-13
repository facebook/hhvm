#define UNUSED(x) \
    ((void)(x))

void exit_on_parent_exit_(int interval, int grace) {
    UNUSED(interval);
    UNUSED(grace);
    return;
}
