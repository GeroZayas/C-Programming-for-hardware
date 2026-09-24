default: vector_table.S main.S
	arm-none-eabi-as -g -o vector_table.o vector_table.S
	arm-none-eabi-as -g -o main.o main.S
	arm-none-eabi-ld -Ttext=0 -Tdata=0x20000000 -g -o STM32VLDiscovery.elf vector_table.o main.o

clean:
	rm -f *.o *.elf