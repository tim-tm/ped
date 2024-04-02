# ped

The **p**erfect **ed**itor or **p**ossibly the worst **ed**itor.
Ped is a vim-like editor made for the terminal using ncurses.

![sc](https://github.com/tim-tm/ped/assets/43402731/bf64a09c-184d-4770-86cd-c22454a87625)

## Getting started

### Package managers

As soon as ped gets usable, it will be available in the AUR. For now it is not available via. any package manager.

### Building on your own

**Prerequisites**

- autotools
- make
- gcc
- pkg-config
- ncurses (listed in pkg-config)

Cloning the repo
```sh
git clone https://github.com/tim-tm/ped.git && cd ped
```

Make the generation script executable (only if it isn't as of right now)
```sh
chmod +x autogen.sh
```

Generating build files
```sh
./autogen.sh
```

Everything needed to build ped will be stored in ./build, autoconf will generate a bunch of files that you shouldn't care that much about.

Building
```sh
make -C build
```
or
```sh
cd build && make
```

The ped executable will be stored in ./build

Installing ped
```sh
sudo make -C build install
```
or
```sh
cd build && sudo make install
```

**Distribution**

If you want to share ped with others, consider creating a tarball.
```sh
make -C build dist
```
or
```sh
cd build && make dist
```

There will now be a tarball named **ped-VERSION.tar.gz** that can be shared with your friends.

Unzipping the tarball
```sh
tar -xf ped-VERSION.tar.gz && cd ped-VERSION
```

Configuring and building ped from the tarball
```sh
./configure && make
```

Installing
```sh
sudo make install
```

## Usage

Ped uses different modes, just like vim or other similar editors do.

| **Mode** | **Purpose**                                                                                                                                                           | **State**             |
|:--------:|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------|
| Normal   | The normal mode is the starting point of the editor,<br>you can navigate around and access every mode from here,<br>take a look at the keyboard shortcuts for more information. | Partially implemented |
| Insert   | As the name implies, the insert mode is made for inserting<br>characters into a buffer (file).                                                                        | Partially implemented   |
| Visual   | The visual mode is useful for selecting and moving bigger<br>pieces of file data.                                                                                     | not yet implemented   |
| Search   | The search mode makes it possible to search inside of<br>buffers (files).                                                                                             | not yet implemented   |

Here is a list of all currently supported keybinds.

|    **Mode**   | **Key** | **Effect**                                                                                                             |
|:-------------:|---------|------------------------------------------------------------------------------------------------------------------------|
| Any           | Escape  | Go into normal mode                                                                                                    |
| Any           | Ctrl+q  | Exit ped                                                                                                               |
| Normal        | j/Down  | Move the cursor down                                                                                                   |
| Normal        | k/Up    | Move the cursor up                                                                                                     |
| Normal        | l/Right | Move the cursor right                                                                                                  |
| Normal        | h/Left  | Move the cursor left                                                                                                   |
| Normal        | a       | Enter insert mode (in vim, 'a' is means **a**ppend<br>and 'i' means **i**nsert, ped is for now only able<br>to append) |
| Normal        | v       | Enter visual mode                                                                                                      |
| Normal        | /       | Enter search mode                                                                                                      |
| Normal        | Ctrl+s  | Save current buffer                                                                                                    |
| Insert        | Down    | Move the cursor down                                                                                                   |
| Insert        | Up      | Move the cursor up                                                                                                     |
| Insert        | Right   | Move the cursor right                                                                                                  |
| Insert        | Left    | Move the cursor left                                                                                                   |
| Insert        | Backspace | Delete the character in front of the cursor                                                                          |
| Insert        | Entf    | Delete the character selected by the cursor                                                                            |
| Insert        | Enter   | Insert an empty line below the cursor                                                                                  |

## Concept

### Ped and storing data

Ped stores file data in form of a 2d-doubly-linked-list. This may sound compilcated but it is a good way of optimizing file operations,
such as adding or removing content from a file.

The general structure looks like this:
```
Character
├── data storage
│   └── value
│       └── The actual character that is being stored (e.g. 'a')
└── references
    ├── next
    │   └── Pointer to the next character
    └── prev
        └── Pointer to the previous character
Line
├── infos
│   └── size
│       └── amount of characters
├── data storage
│   ├── chars
│   │   └── every character of a line
│   ├── first_char
│   │   └── first char of a line
│   └── last_char
│       └── last char of a line
└── references
    ├── next
    │   └── Pointer to the next line
    └── prev
        └── Pointer to the previous line
Buffer
├── infos
│   ├── filename
│   │   └── Name of the buffer (file) being read from/being written to
│   └── size
│       └── amount of lines
└── data storage
    ├── lines
    │   └── every line of the buffer (file)
    ├── first_line
    │   └── the first line
    └── last_line
        └── the last line
```

Example:

> test.txt
> ```
> hi!
> qa!
> ```

The [serialized](https://en.wikipedia.org/wiki/Serialization) data would look like this:

```
Buffer
├── filename
│   └── "test.txt"
├── size
│   └── 2
├── lines
│   ├── size
│   │   └── 4
│   ├── chars
│   │   ├── 'h'
│   │   ├── next
│   │   │   ├── 'i'
│   │   │   ├── next
│   │   │   │   ├── '!'
│   │   │   │   ├── next
│   │   │   │   │   └── none
│   │   │   │   └── prev
│   │   │   │       ├── 'i'
│   │   │   │       └── ...
│   │   │   └── prev
│   │   │       ├── 'h'
│   │   │       └── ...
│   │   └── prev
│   │       └── none
│   ├── first_char
│   │   ├── 'h'
│   │   ├── next
│   │   │   └── ...
│   │   └── prev
│   │       └── none
│   ├── last_char
│   │   ├── '!'
│   │   ├── next
│   │   │   └── none
│   │   └── prev
│   │       └── ...
│   ├── next
│   │   ├── size
│   │   │   └── 4
│   │   ├── chars
│   │   │   ├── 'q'
│   │   │   ├── next
│   │   │   │   ├── 'a'
│   │   │   │   ├── next
│   │   │   │   │   ├── '!'
│   │   │   │   │   ├── next
│   │   │   │   │   │   └── none
│   │   │   │   │   └── prev
│   │   │   │   │       ├── 'a'
│   │   │   │   │       └── ...
│   │   │   │   └── prev
│   │   │   │       ├── 'q'
│   │   │   │       └── ...
│   │   │   └── prev
│   │   │       └── none
│   │   ├── first_char
│   │   │   ├── 'h'
│   │   │   ├── next
│   │   │   │   └── ...
│   │   │   └── prev
│   │   │       └── none
│   │   ├── last_char
│   │   │   ├── '!'
│   │   │   ├── next
│   │   │   │   └── none
│   │   │   └── prev
│   │   │       └── ...
│   │   └── next
│   │       └── none
│   └── prev
│       └── none
├── first_line
│   ├── size
│   │   └── 4
│   ├── chars
│   │   ├── 'h'
│   │   ├── next
│   │   │   └── ...
│   │   └── prev
│   │       └── none
│   ├── first_char
│   │   ├── 'h'
│   │   ├── next
│   │   │   └── ...
│   │   └── prev
│   │       └── none
│   ├── last_char
│   │   ├── '!'
│   │   ├── next
│   │   │   └── none
│   │   └── prev
│   │       └── ...
│   ├── next
│   │   └── ...
│   └── prev
│       └── none
└── last_line
    ├── size
    │   └── 4
    ├── chars
    │   ├── 'q'
    │   ├── next
    │   │   └── ...
    │   └── prev
    │       └── none
    ├── first_char
    │   ├── 'q'
    │   ├── next
    │   │   └── ...
    │   └── prev
    │       └── none
    ├── last_char
    │   ├── '!'
    │   ├── next
    │   │   └── none
    │   └── prev
    │       └── ...
    ├── next
    │   └── ...
    └── prev
        └── none
```

This way of storing file-data comes with advantages and disadvantages though.
It is easier to mainipulate, insert and append buffer content (Lines, Chars) because you only need to change references.
The main downside is the serialization and deserialization, that process is way more complex than just copying a string,
which is what you would do if you were to use an array of strings.

Manipulation example:

> Original text
> ```
> abc
> ```
> Pointer structure: a&rarr;b&rarr;c

> Should be changed to
> ```
> ac
> ```
> Pointer structure: a&rarr;b&rarr;c

In order to change the original text to the new text, we only need to point 'a' to 'c' instead of pointing it to 'b' and 'b' pointing to 'c'.
If we stored our data in a 2d-array, we would need to erase 'b' and move every character after 'b' to the left by one in order to fill gaps.

Finding the right character inside of the linked-list takes O(n) time at most, accessing an array takes a constant amount of time (O(1)).
Moving memory however is a expensive operation that needs to be done for n characters at most.
Changing the reference of a character only needs to be done once.

Even though linked-lists are faster to manipulate, this is a choice of preference.
It is definetly much more simple to store buffer data in an array of strings.

## Contributing

Steps to contribution:

1. Make a fork of ped
2. create a feature branch (optional, may be done on bigger changes tho)
3. Add **and test** your changes inside of your fork
4. Open a pull request if you're happy with your contribution

Your changes should not be extremely huge, if you really want to contribute something that may break the editor or introduce a dramatical change, please open an issue first in order to discuss your ideas.

## License

Ped is licensed under the [MIT License](https://github.com/tim-tm/ped/blob/main/LICENSE).
