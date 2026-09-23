# Copy this to the terminal for easy aliases:
# alias r="make run" alias b="make build" alias c="clear"
# c && b && r

build:
	gcc main.c -o main -std=c99 -Wall -Werror -pedantic

debug:
	gcc main.c -o main -g -std=c99 -Wall -Werror -pedantic

run:
	@clear && ./main

buildrun: build run

m ?=update
git:
	git add -A && git commit -m "$(m)"
	
push:
	git push

clean:
	rm -rf main.DSYM main 