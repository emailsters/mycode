alias ll='ls -l'
export PS1='\u@\h:\w> '

if [ -f /etc/bashrc ]
then
    . /etc/bashrc
fi