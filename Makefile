subdirs = kmod test

all:
	for i in $(subdirs); do \
		echo; echo $$i; \
		make -C $$i; \
	done

insmod:
	make -C kmod insmod

rmmod:
	sudo rmmod psock

run:
	make -C test run

clean:
	for i in $(subdirs); do \
		echo; echo $$i; \
		make -C $$i clean; \
	done
