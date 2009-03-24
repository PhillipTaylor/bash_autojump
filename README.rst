
-- PROJECT STATUS --

This code isn't quite working yet. We suggest you look for other implementations in the meantime.

Original Python implementation: http://github.com/joelthelion/autojump/tree/master
Later BASH script implementation: http://github.com/KenMacD/autojump/tree/master

-- INTRO --

bash_autojump is an implementation of the autojump functionality but written as a patch that can be applied to the BASH 4.0 source code. Below is the original README which describes all of the features.

This specific reimplementation was written by Dan Kendall and Phillip Taylor.

========
AUTOJUMP
========

----------------------------
A ``cd`` command that learns
----------------------------

One of the most used shell commands is ``cd``. A quick survey among my friends revealed that between 10 and 20% of all commands they type are actually ``cd`` commands! Unfortunately, jumping from one part of your system to another with ``cd`` requires to enter almost the full path, which isn't very practical and requires a lot of keystrokes.

autojump is a faster way to navigate your filesystem. It works by maintaining a database of the directories you use the most from the command line. The jumpstat command shows you the current contents of the database. You need to work a little bit  before  the  database becomes useable. Once your database is reasonably complete, you can "jump" to a directory by typing::

 j dirspec

where dirspec is a few characters of the directory you want to jump to. It will jump to the most used  directory  whose
name matches the pattern given in dirspec.

Autojump supports tab completion. Try it!

Examples
========

::

 j mp3

could jump to ``/home/gwb/my mp3 collection``, if that is the directory in which you keep your mp3s. ::

 jumpstat</b>

will print out something in the lines of::

 ...
 54.5:	/home/shared/musique
 60.0:	/home/joel/workspace/coolstuff/glandu
 83.0:	/home/joel/workspace/abs_user/autojump
 96.9:	/home/joel/workspace/autojump
 141.8:	/home/joel/workspace/vv
 161.7:	/home/joel
 Total key weight: 1077

The "key weight" reflects the amount of time you spend in a directory.

Special Thanks to the original authors of Autojump
======

Joel Schaerer (joel.schaerer (at) laposte.net)
Install script written by Daniel Jackoway

(You should contact Dan Kendall or Phillip Taylor, see top of readme, for information about this specific implementation of Autojump)

