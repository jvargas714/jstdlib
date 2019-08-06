#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/time.h>

/*
    todo :: later expand this to take write_fds as well
 */
class fd_sets {
    private:
        fd_set working_set;		// set that is currently being processed 
        fd_set master_set;		// backed up set of all connections 
        int max_fd;
        struct timeval timeout;
    
    public:
        fd_sets();

        // back up master
        void set_working_set();

        // wrapper around select()
        int select_set(bool readfds=true);


        // return vector containing active fds from working set 
        // returns empty vector if none are active 
        std::vector<int> get_active_fds() const;

        // adds an fd to the mastef fd set
        void add_fd(int fd);

        // clears working and master set 
        void clear();

        // set timeout in millisecs
        void set_timeout_ms(long millisecs);

        // remove descriptor from the master set
        void clear_fd(int fd);

};

