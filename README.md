# TAMU 611 Operating System projects

## Build

In each MP folder, run the following command.

```bash
make
```

## Run

### Prerequisite

```bash
sudo mkdir /mnt/floppy
```

In each MP folder (except MP1), run the following commands.

1. Update the floppy image

```bash
cp -r ../MP1/dev_kernel_grub.img ./
./copyimage.sh
# cat ./copyimage.sh for more details
```

2. Run in a hypervisor (Bochs, Virtualbox, qemu, etc.)

* Mount the dev_kernel_grub.img

* Start VM.

## Documents

The design.pdf file in each MP folder explains some design and implementation details for the project.  The description of each machine problem can be found [here](https://people.engr.tamu.edu/bettati/Courses/410/2010C/Projects/project_overview.html).
