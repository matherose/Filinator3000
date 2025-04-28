# 🤖 FILINATOR 3000 🤖

**Filinator** is a C program so minimalist it makes Marie Kondo look like a compulsive hoarder. This little C89 marvel transforms your file and directory names with the elegance of a ninja and the efficiency of a German kitchen appliance. It's the only relationship with transformation that most programmers will ever experience.

## First Things First... why?

Everything started from a statement : ***1fichier*** (my file hoster) don't like uploading folders through FTP... yep that's it.
So I needed a way to encode my folders and files names to make them compatible with.
I wanted to make it as simple as possible, so I wrote this little program.
I also wanted to make it as fun as possible, so I added a lot of useless features and a lot of useless comments.


## ✨ How This Beast Actually Works ✨

Filinator operates in two primary modes, each capable of mangling your file system in its own special way:

### 1. Encode Mode
```
./filinator -encode <directory> [-output <directory>]
```
- **Directory Encoding**: Spaces in folder names become section characters (`§`). Because spaces are so mainstream.
- **File Encoding**: 
  - `/` becomes `@` (slashes are *so* divisive)
  - Spaces become `_` (we don't do spaces in Filinator-land)
  - Section characters (`§`) become spaces (plot twist!)

### 2. Decode Mode
```
./filinator -decode <directory>
```
- Reverses all the encoding transformations, returning your beautifully mangled filenames back to their boring original state.

### The Secret Sauce

Filinator recursively processes directories faster than you can say "why am I doing this to my files?" It can either:
- Rename files in place (the digital equivalent of reorganizing your sock drawer while blindfolded)
- Copy to an output directory with transformed names (creating a parallel universe where your files have alter-egos)

If you don't specify an output directory when encoding, Filinator helpfully creates a folder called "output" because it's just that thoughtful.

## ✨ Extraordinary Features ✨

- **In-Place Encoding** (aka "I-Touch-Everything-Mode"):  
  Transform your spaces into special characters (`§`) more easily than your cat transforms your couch into a scratching post. For files, we replace `/` with `@`, spaces with `_`, and `§` with spaces. Why? Because we can! Much like how you "can" install Arch Linux from scratch, but it doesn't mean you should. 🎩

- **Output Mode** (aka "Copies-Everywhere-Mode"):  
  Use the `-output` flag and watch Filinator copy your files like a compulsive hamster, creating a "flattened" structure flatter than your uncle's jokes at family dinners. It's like what happens to your social life after discovering Stack Overflow—suddenly everything exists in one dimension.

- **Decoding** (aka "Oops-I-Made-A-Mistake-Mode"):  
  Reverse all these brilliant transformations when you realize that nobody understands your file system anymore. Back to square one, but with style! It's the digital equivalent of admitting you've been pronouncing "GIF" wrong all these years.

## 🛠️ Compilation for the Brave

Filinator is written in C89, which is a bit like saying it uses the Latin of programming languages, but with fewer people to impress at parties. To compile it with GCC and benefit from optimizations (because saving 0.002 seconds on execution is life-changing when you spend 4 hours arguing about tab vs. spaces), run:

```bash
gcc -std=c89 -O2 -D_POSIX_C_SOURCE=200809L -o filinator filinator.c
```

Or just use the provided Makefile like a sensible human being:

```bash
make
```

If you understand all these flags, congratulations! You're officially too cool for this planet and probably haven't seen sunlight since 2019. 🚀

## 🧠 Technical Deep Dive (For The Truly Desperate) 🧠

Filinator performs its magic through recursive directory traversal, which is just a fancy way of saying it goes through your folders like an overeager toddler exploring cupboards. During this adventure, it:

1. Processes each directory before renaming it (so it can find all the files first)
2. Creates any missing directories during decoding (because it's not a monster)
3. Handles absolute and relative paths like a geography professor with a GPS
4. Copies files buffer-by-buffer when in output mode (4KB at a time, for those counting)
5. Carefully avoids transforming "." and ".." directory entries (because even chaos has limits)

Technically, this program could handle any character encoding that fits within PATH_MAX (4096) characters, which should be enough for anyone except people who name their files like they're writing a novel.

## 💼 POSIX Compliance & Portability 💼

Filinator has been carefully crafted to be as portable as a pocket calculator:

- Sticks to C89 standard (released when dinosaurs still roamed the Earth)
- Uses only POSIX-compliant functions that work across Unix-like systems
- Avoids non-standard extensions like a programmer avoids sunlight
- Handles file paths with more care than most people handle their retirement savings
- Works on Linux, macOS, and any system where GCC can be convinced to run

It's so portable that archaeologists will be able to compile it thousands of years from now, assuming they've discovered electricity.

## 🤓 Real-World Applications 🤓

Filinator works wonders on your media collections, turning file names like `MySuperMovieMadeWithTarantinoAndMySisterIn2004BeforeTheSecondKillBillMovieAndBeforeItWasCool.MP4` into something even more unreadable! Perfect for:

- People whose folder organization is best described as "digital hoarding but make it technical"
- Developers who want to make sure absolutely no one else can understand their file structure
- Those who think file naming conventions are too mainstream and prefer to live dangerously
- Anyone who needs plausible deniability for their questionable media organization skills
- Individuals who enjoy creating filenames that resemble cryptic puzzles

## 🚨 WARNING 🚨

Using Filinator may cause extreme confusion among your colleagues, blind admiration from computer nerds, and filenames that even their creator won't recognize anymore. Side effects include involuntary snorting at your own terminal, explaining your file system to confused coworkers for hours, and developing an unhealthy emotional attachment to special characters. Use with moderation and an intact sense of humor.

*Note: No actual nerds were harmed in the making of this README, though several egos were slightly bruised.*
