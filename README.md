# obj-io
This repository contains a single-file, header-only, no-dependencies C++ implementation of the [OBJ file format](https://en.wikipedia.org/wiki/Wavefront_.obj_file). All code in this repository is released under the [MIT license](https://en.wikipedia.org/wiki/MIT_License), as per the included [license file](https://github.com/thinks/obj-io/blob/master/LICENSE).

## The OBJ File Format
The [OBJ file format](https://en.wikipedia.org/wiki/Wavefront_.obj_file) is commonly used throughout the field of computer graphics. While it is arguably not the most efficient way to store meshes on disk, the fact that it is widely supported has made it ubiquitous. In this sense the OBJ file format is extremely useful for debugging and transferring meshes between software packages.

## Examples
Mesh representations vary wildly across different frameworks. It seems fairly likely that most frameworks have their own representation. Because of this, the methods provided for reading and writing OBJ files assume no knowledge of a mesh class. Instead, the methods rely on callbacks provided by users that provide the methods with the required information. As such, the methods act more as middle-ware than some out-of-the-box solution. While this approach requires some additional work for users, it provides great flexibility and arguably makes this distribution more usable in the long run.

A simple example illustrates how to read and write a mesh using our method.


## Tests

## Future Work

* vertices must be same size, 3 or 4 elements.
* all primitives must be same size, greater than or equal to 3, triangle and up.
* does not support materials.

