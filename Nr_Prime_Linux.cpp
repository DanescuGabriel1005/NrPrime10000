#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

bool isPrime(int n) {
    if (n < 2) return false;
    for (int d = 2; d * d <= n; ++d)
        if (n % d == 0) return false;
    return true;
}

int main() {
    const int NUM_PROCS = 10;
    const int RANGE = 1000;

    int fromChild[NUM_PROCS];
    pid_t pids[NUM_PROCS];

    for (int i = 0; i < NUM_PROCS; ++i) {
        int toChildPipe[2];
        int fromChildPipe[2];
        if (pipe(toChildPipe) == -1 || pipe(fromChildPipe) == -1) {
            perror("pipe");
            return 1;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            close(toChildPipe[1]);
            close(fromChildPipe[0]);

            int interval[2];
            if (read(toChildPipe[0], interval, sizeof(interval)) != sizeof(interval)) {
                _exit(1);
            }
            close(toChildPipe[0]);

            vector<int> primes;
            for (int n = interval[0]; n <= interval[1]; ++n)
                if (isPrime(n)) primes.push_back(n);

            if (!primes.empty())
                write(fromChildPipe[1], primes.data(), primes.size() * sizeof(int));
            close(fromChildPipe[1]);
            _exit(0);
        }

        close(toChildPipe[0]);
        close(fromChildPipe[1]);

        int interval[2] = { i * RANGE, (i + 1) * RANGE - 1 };
        write(toChildPipe[1], interval, sizeof(interval));
        close(toChildPipe[1]);

        fromChild[i] = fromChildPipe[0];
        pids[i] = pid;
    }

    for (int i = 0; i < NUM_PROCS; ++i) {
        cout << "Procesul " << i << " (" << i * RANGE << " - " << (i + 1) * RANGE - 1 << "):\n";
        int p;
        while (read(fromChild[i], &p, sizeof(p)) == sizeof(p))
            cout << p << " ";
        cout << "\n\n";
        close(fromChild[i]);
        waitpid(pids[i], nullptr, 0);
    }
    return 0;
}
