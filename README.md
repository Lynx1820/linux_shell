Names: Susan Hao and Keren De Jesus
eniac usernames: susanhao and dkeren

COMPILATION INSTRUCTIONS
type 'make clean'
type 'make'
type './seashell'
YAY YOU CAN USE IT!!!!

SOME THINGS TO NOTE WHEN RUNNING OUR PROGRAM
-when you cat something (ie cat > file.txt)
-after you type in the file, you need to press 'enter'
-then you can press 'ctrl-d'
-then you need to press 'enter' again

OVERVIEW OF WORK
-we implemented redirect, piping, background, and foreground processes, terminal control, and job ids
-it works very similarly to a normal linux shell


DESCRIPTION OF CODE/CODE LAYOUT
-our code is divided into 4 parts: handler, get_tokens, parent_child, and main
-the handler handles all the signals delivered to it
-get_tokens uses token-shell to parse the command from terminal
-parent_child deals with forking, terminal contorl, and parent and child processes
-main wraps it all