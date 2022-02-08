# badCBOR

convert a hex string to a CBOR-formatted (json-like) string

## Setup

Compile
```
gcc -Wall -pedantic parse.c -o parse
```

## Example

Example input :
```
./parse A4636B65791901F402A1038304050663666F6F6362617281074101
```

Out should be :
```
CBOR : <{"key" : 500, 2 : {3 : [4, 5, 6]}, "foo" : "bar", [7] : '0b01000001'}>
```
