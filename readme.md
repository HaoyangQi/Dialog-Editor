# Dialog Editor

## Project **dlg-create**

This is a test project for dialog resource interpretation.

![example dialog resource dump](./image/resource-analyzed.png)
The dump file is originated from the default dialog box resource by Visual Studio 2019. The blue region contains 2 resource headers (1 for whole file, 1 for the following dialog resource), each has 32 bytes. Red, Orange (title string), Green (typeface string) region combined is the dialog template data. Brown region is 2-byte padding, following Purple region shows few fields at the beginning of a dialog control item, more fields come afterwards.

### DWORD Alignment

All data structures are organized by aligning on DWORD (4-byte) boundary, both in file and in memory.

The size of data can be calculated by setting a dummy start address as `0x0`, which is DWORD (4-byte) aligned.

In order to load data into memory: allocate memory, then align pointer to the first DWORD boundary, and copy over the data.

Standard n-byte alignment is expressed as following:
```
aligned = ((addr + n - 1) / n) * n
```
where the division only keeps integer part.

For DWORD alignment, the division and multiplication can be optimized by left/right shift by 2:
```
aligned = ((addr + 3) >> 2) << 2
```

### WORD Alignment

All length-varying data (string, creation data) are encoded in UNCODE, and they are aligned on WORD boundary.

The first bit (LSB) is enough to determine the size of padding (0 or 1 byte):
```
aligned = addr + addr & 1
```

### Dialog Template

A dialog starts with a data structure `DLGTEMPLATE(EX)`, and `DLGTEMPLATEEX` is not well-defined because size of some fields are runtime-determined.

Followed after `DLGTEMPLATE(EX)`, are multiple `DLGITEMTEMPLATE(EX)` structures and each one of them are DWORD-aligned. `DLGITEMTEMPLATEEX` is not well-defined due to same reason above.

## Project **application-test**

This project tests critical UI functionalities

### Window Designer
