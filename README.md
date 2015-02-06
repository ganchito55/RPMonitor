# RPMonitor
The lightest remote monitor for Raspberry Pi, written in C, with no dependencies and fully hackable and customizable.

## How it works

The project has 3 files:
* the server.c file, with all code about create a TCP connection, and send the HTML code with the monitor values through Internet.
* the monitor.html with the user interface, it is fully customizable. The server reads this file and then it send it, but it replaces the variables by the results.
* the vars file with all variables and how to resolve them. It use the following format $var$'TAB'command

Manage shell script is only for developers and people who want to change and test the server

## How to build

You need gcc and other developer tools. I will try to upload the last build for non developers. I think that if you have installed build-essential you won't need any dependencies more.

Compile the file server.c

$ gcc server.c -o server

## How to install and configure

You only have to run server ($ ./server ). You can change de variables and add the information that you want to monitor.

## How to use

When server is running, with a web browser enter yourRaspberryIP:5555 and you can show all the information. Example:

## Variables and more

I try do not use third party software because I think that the software should be run without other dependencies, so I hope that others will follow this advice. But this is only an advice.

Help me to increase the vars file with all variables that you think that it would be interesting to monitor!!

## Thanks

The code to create the server and manage a TCP connection was obtained from [he
re](http://www.kumanov.com/docs/prog/Advanced%20Systems%20Programming%20And%20Real%20-%20Time%20Systems%20&%20Real-%20Time%20Operating%20Systems%20And%20Device%20Programming/RTGIF031.HTM)
Icons from [Font Awesome](http://fortawesome.github.io/Font-Awesome/)
