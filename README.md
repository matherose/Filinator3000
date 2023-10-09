
# Filinator3000

I created this script, because I wanted to backup my NAS, but my host (hello 1fichier) only accepts **files**, so instead of bothering to create all my folders manually, I bothered even more to create a script to do that automatically, before an (S)FTP upload.
(I have too much free time...)

I recommend to move the script to an environment present in the path, like /usr/local/bin, and then to be used in the folder you want.

For example:
You want to save your **stuff_super_important** folder
```bash
$ ls -l stuff_super_important
  | - latepayment.txt
  | - nothingimportant/onlyfansbill.xlsx
  | - nothingimportant/last night with mistress wife must not discover.png
```

Go to the folder and launch the script to encode
```bash
$ cd stuff_super_important
$ filinator.sh --encode
```
**Result**
```bash
$ ls -l
  | - latepayment.txt
  | - nothingimportant@onlyfansbill.xlsx
  | - nothingimportant@last§night§with§mistress§wife§must§not§discover.png
```
And you're ready to go !

And once your wife has gone to work, you can download your files back to the office PC to restore everything:
```bash
$ cd [Where_you_want_to_decode] (Import the file here)
$ filinator.sh --decode
```

```bash
$ cd stuff_super_important

$ ls -l
  | - latepayment.txt
  | - nothingimportant/onlyfansbill.xlsx
  | - nothingimportant/last night with mistress wife must not discover.png
```
