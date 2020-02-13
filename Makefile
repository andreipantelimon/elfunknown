build:
		gcc -Wall -Wextra -std=c99 main.c -o snowfight -lm

clean :
		rm -f snowfight *~