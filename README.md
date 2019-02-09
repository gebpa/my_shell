# my_shell
Command interpreter. Written on C. It is practice to work with system calls   
   
This command interpreter has following functionality  
1) pipelines with multiple processes (more than 2 processes)  
if built-in is a part of pipeline, the built-in does not execute, and input of the next process in pipeline will be NULL  
example:  
$ ls | cd .. | wc  
0  
working directory will not be changed and wc will get as input nothing  
2) create variables  
you can create local variables  
examples:  
$ hello=world  
$ echo $hello  
world  
$ bat=man red=alert  
$ echo $red spider$bat  
alert spiderman  
$ bat=man ls  
dir1 dir2 file1 file2  
$ echo  
$bat  
3) export variables   
you can export variables  
and they will exist in child processes  
$ metal=gear  
$ export solid=snake hello=world metal  
$ serious=sam export serious  
$ sh  
$ echo $serious $solid $hello $metal  
sam snake world gear  
# my_shell
