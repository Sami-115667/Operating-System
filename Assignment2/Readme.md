
#### Run the Python server:
- Write the current update version inside the file `version.txt`
- Specify the path to the current OS version binary inside the file `file_path.txt`
- Run the Python Server using command:
```bash
python main_server.py
```

#### Load the Bootloader Program into the device following these steps:
- Navigate to the `Bootloader/src/compile` directory.  
- Open that location in the command line.  
- Use the command `make all` to create the binary file.  
- Use the command `make load` to load the binary file onto the STM32F446RE MCU.  
