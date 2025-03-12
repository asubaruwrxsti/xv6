#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
    int pipe1[2]; // Parent -> Child pipe
    int pipe2[2]; // Child -> Parent pipe

    // Create pipes
    // Pipes serve as a communication channel between two processes
    // They are unidirectional, so we need two pipes for bidirectional communication
    if (pipe(pipe1) < 0 || pipe(pipe2) < 0)
    {
        perror("pipe creation failed");
        exit(1);
    }

    pid_t pid = fork();
    printf("Process %d: fork() returned %d\n", getpid(), pid);

    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0)
    {
        printf("I am the child (PID %d), and my pid variable is %d\n", getpid(), pid);
        // Child process
        close(pipe1[1]); // Close write end of pipe1
        close(pipe2[0]); // Close read end of pipe2

        char byte;
        while (1)
        {
            // Receive ping
            if (read(pipe1[0], &byte, 1) != 1)
                break;
            printf("Child received: %c\n", byte);

            // Send pong
            byte = 'O';
            if (write(pipe2[1], &byte, 1) != 1)
                break;
            printf("Child sent: %c\n", byte);
            sleep(1);
        }

        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);
    }
    else
    {
        printf("I am the parent (PID %d), and my pid variable is %d\n", getpid(), pid);
        // Parent process
        close(pipe1[0]); // Close read end of pipe1; parent writes to pipe1
        close(pipe2[1]); // Close write end of pipe2; parent reads from pipe2

        char byte;
        for (int i = 0; i < 5; i++)
        {
            // Send ping
            byte = 'I';
            if (write(pipe1[1], &byte, 1) != 1)
                break;
            printf("Parent sent: %c\n", byte);

            // Receive pong
            if (read(pipe2[0], &byte, 1) != 1)
                break;
            printf("Parent received: %c\n", byte);
            sleep(1);
        }

        close(pipe1[1]);
        close(pipe2[0]);
        wait(NULL);
    }

    return 0;
}