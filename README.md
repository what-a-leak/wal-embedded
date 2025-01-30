# wal_embedded

EPS32C3 based node embedded software for What a Leak project. 


## Todo List

- [x] Gather raw microphone data
- [x] Perform FFT on microphone data
- [x] Construct Payload
- [x] Transmit Payload via LoRa

## Choose the mode of the node

Simply modify the [`platformio.ini`](platformio.ini) file definition for the following line:

### Disable/Enable Encryption Tests
```
build_flags = 
	-I include
	-D DISABLE_SECURITY
```

### Master Mode
```
build_flags = 
	-I include
	-D MASTER_NODE
```

### Node Mode
```
build_flags = 
	-I include
```
