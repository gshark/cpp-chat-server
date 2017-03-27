#ifndef FD_CLOSER_H
#define FD_CLOSER_H


class fd_closer
{
private:
    //int fd;
public:
    fd_closer();
    fd_closer(int fd);
    fd_closer(const fd_closer&) = delete;
    fd_closer& operator = (const fd_closer&) = delete;

    fd_closer(fd_closer&&);
    fd_closer& operator = (fd_closer&&);


    ~fd_closer();
    int get_fd() const;
    int fd;
};

#endif // FD_CLOSER_H
