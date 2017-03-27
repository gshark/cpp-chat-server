#ifndef FD_CLOSER_H
#define FD_CLOSER_H


class fd_closer
{
public:
    fd_closer();
    fd_closer(int fd);
    ~fd_closer();
    int fd;
};

#endif // FD_CLOSER_H
