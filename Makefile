
all:
	platformio run

clean:
	platformio run --target clean

upload:
	platformio run --target upload 

.PHONY: all clean upload
