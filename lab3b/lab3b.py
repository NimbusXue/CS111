# NAME: JIAXUAN XUE,Felix Gan
# EMAIL: nimbusxue1015@g.ucla.edu,felixgan@g.ucla.edu
# ID: 705142227,205127385

import sys
import csv
reserved = {0, 1, 2, 3, 4, 5, 6, 7, 64}
block_sum = 0
inode_sum = 0
corrupted_flag = 0
offset = 0
links = 0
block = dict()
free_block = set()
parent_to_inode = dict()    
inode_to_parent = dict()
inode_ref_link_count = dict()
free_inode = set()
inode_link_count = dict()
inode_ref = dict()


def read_CSV():
    def print_error(error, indirections, num_b, num_i, num_offset):
        global corrupted_flag
        corrupted_flag = 1
        msg = "{}{} BLOCK {} IN INODE {} AT OFFSET {}".format(error,
                                                               indirections,
                                                               num_b,
                                                               num_i,
                                                               num_offset)
        print(msg)

    def save_to_block(num_b, num_i, offset, num_indirections):
        if num_b in block:
            block[num_b].append([num_i, offset, num_indirections])
            return
        block[num_b] = [[num_i, offset, num_indirections]]

    def scan_row(row):
        global reserved, block_sum, inode_sum, corrupted_flag, offset, links, block, free_block, parent_to_inode, inode_to_parent, inode_to_parent, inode_ref_link_count, free_inode, inode_link_count, inode_ref
        title = row[0]

        if title == 'INDIRECT':
            inode_index, num_indirections, level_offset, num, block_cur = int(
                row[1]), int(row[2]), int(row[3]), int(row[4]), int(row[5])
            indirect_type = ['', ' INDIRECT',
                             ' DOUBLE INDIRECT', ' TRIPLE INDIRECT']
            offset = [0, 12, 268, 65804]

            if block_cur > block_sum or block_cur < 0:
                print_error(
                    "INVALID", indirect_type[num_indirections], block_cur, inode_index, offset[num_indirections])
            elif block_cur != 0 and block_cur in reserved:
                print_error(
                    "RESERVED", indirect_type[num_indirections], block_cur, inode_index, offset[num_indirections])
            else:
                save_to_block(block_cur, inode_index,
                              offset[num_indirections], num_indirections)

        elif title == 'DIRENT':
            inode_num, byte_offset, dir_inode, rec_len, name_len, name = int(
                row[1]), int(row[2]), int(row[3]), int(row[4]), int(row[5]), row[6]

            inode_ref_link_count[dir_inode] = 1 if dir_inode not in inode_ref_link_count else inode_ref_link_count[dir_inode] + 1

            inode_ref[dir_inode] = name
            name_raw = name[1:-1]

            if dir_inode > inode_sum or dir_inode < 1:
                msg = 'DIRECTORY INODE {} NAME {} INVALID INODE {}'.format(
                    inode_num, name[0:len(name)], dir_inode)
                print(msg)
                corrupted_flag = 1

            if name_raw == '..':
                parent_to_inode[inode_num] = dir_inode
            elif name_raw == '.':
                if dir_inode != inode_num:
                    msg = 'DIRECTORY INODE {} NAME {} LINK TO INODE {} SHOULD BE {}'.format(
                        inode_num, "'.'", name[0:len(name)], inode_num)
                    print(msg)
                    corrupted_flag = 1
            else:
                # put it in the reverse list
                inode_to_parent[dir_inode] = inode_num

        elif title == 'IFREE':  # put in free inodes list
            count = int(row[1])
            free_inode.add(count)

        elif title == 'BFREE':  # put in free blocks list
            count = int(row[1])
            free_block.add(count)

        elif title == 'INODE':
            inode_index, i_type, i_mode_masked, i_uid, i_gid, i_links_count, i_ctime, i_mtime, i_atime, i_size, i_blocks = int(
                row[1]), row[2], int(row[3]), int(row[4]), int(row[5]), int(row[6]), row[7], row[8], row[9], int(row[10]), int(row[11])
            inode_link_count[inode_index] = i_links_count

            if len(row) < 27:
                sys.exit("Inode row too short. Expected 27, got " +
                         len(row) + ".")        # would inode go out of range?
            for i in range(12, 27):
                num = int(row[i])
                if num == 0:
                    continue

                num_indirections = i-23 if i >= 24 and i <= 26 else 0
                indirect_type = ['', ' INDIRECT',
                                 ' DOUBLE INDIRECT', ' TRIPLE INDIRECT']
                offset = [0, 12, 268, 65804]

                if num < 0 or num > block_sum:
                    print_error(
                        "INVALID", indirect_type[num_indirections], num, inode_index, offset[num_indirections])
                elif num != 0 and num in reserved:
                    print_error(
                        "RESERVED", indirect_type[num_indirections], num, inode_index, offset[num_indirections])
                else:
                    save_to_block(num, inode_index,
                                  offset[num_indirections], num_indirections)

        elif title == 'SUPERBLOCK':
            block_sum, inode_sum = int(row[1]),  int(row[2])
        elif title == 'GROUP':
            pass            # We will not corrupt superblock or group descriptor table
        else:
            print(title)
            sys.exit("Unrecognized Data")

    if len(sys.argv) != 2:
        sys.exit("Incorrect number of arguments: expected 2, got " +
                 repr(len(sys.argv)) + ".")

    file_name = sys.argv[1]

    try:
        file = open(file_name, 'r')
    except FileNotFoundError as error:
        sys.exit(error)
    except:
        sys.exit("Error: File cannot be opened")

    data = csv.reader(file, delimiter=',')
    for row in data:
        scan_row(row)

    file.close()


def find_unreferenced():
    global block_sum, block, reserved, free_block, corrupted_flag
    i = 1
    # print("block sum is ")
    # print(block_sum)
    # print("block is ")
    # for key , value in block.items():
    #     print(key,value)
    # print("free block is")
    # print(free_block)
    # print("reserved block is ")
    # print(reserved)
    while i < block_sum + 1:
        if i not in block and i not in reserved and i not in free_block:
            print('UNREFERENCED BLOCK ' + str(i))
            corrupted_flag = 1
        elif i in free_block and i in block:
            print('ALLOCATED BLOCK ' + str(i) + ' ON FREELIST')
            corrupted_flag = 1
        i += 1


def find_unallocated_freelist():
    global inode_sum, free_inode, inode_link_count, inode_to_parent, corrupted_flag
    set1 = {1, 3, 4, 5, 6, 7, 8, 9, 10}
    i = 1
    while i < inode_sum + 1:
        if i not in free_inode and i not in inode_link_count and i not in inode_to_parent:
            if i not in set1:
                print('UNALLOCATED INODE ' + str(i) + ' NOT ON FREELIST')
                corrupted_flag = 1
        elif i in free_inode and i in inode_link_count:
            print('ALLOCATED INODE ' + str(i) + ' ON FREELIST')
            corrupted_flag = 1
        i += 1


def find_duplicate_blocks():
    global block, corrupted_flag
    for key, value in block.items():
        if len(value) > 1:
            corrupted_flag = 1
            for i in value:
                if int(i[2]) == 0:
                    print('DUPLICATE' + ' BLOCK ' + str(key) +
                          ' IN INODE ' + str(i[0]) + ' AT OFFSET ' + str(i[1]))
                elif int(i[2]) == 1:
                    print('DUPLICATE' + ' INDIRECT' + ' BLOCK ' + str(key) +
                          ' IN INODE ' + str(i[0]) + ' AT OFFSET ' + str(i[1]))
                elif int(i[2]) == 2:
                    print('DUPLICATE' + " DOUBLE INDIRECT" + ' BLOCK ' + str(key) +
                          ' IN INODE ' + str(i[0]) + ' AT OFFSET ' + str(i[1]))
                elif int(i[2]) == 3:
                    print('DUPLICATE' + " TRIPLE INDIRECT" + ' BLOCK ' + str(key) +
                          ' IN INODE ' + str(i[0]) + ' AT OFFSET ' + str(i[1]))


def check_dir_link():
    global parent_to_inode, corrupted_flag, inode_to_parent
    for p, inode in parent_to_inode.items():
        if p==2 and inode == 2:
            continue
        if p == 2 and inode != 2:
            print("DIRECTORY INODE 2 NAME '..' LINK TO INODE " +
                  str(inode) + " SHOULD BE 2")
            corrupted_flag = 1
        elif p not in inode_to_parent:
            print("DIRECTORY INODE " + str(inode) +
                  " NAME '..' LINK TO INODE " + str(p) + " SHOULD BE " + str(inode))
            corrupted_flag = 1
        elif inode != inode_to_parent[p]:
            print("DIRECTORY INODE " + str(p) + " NAME '..' LINK TO INODE " +
                  str(p) + " SHOULD BE " + str(inode_to_parent[p]))
            corrupted_flag = 1


def check_link_count():
    global inode_link_count, inode_ref_link_count, corrupted_flag
    for inode, link_count in inode_link_count.items():
        links = 0
        if inode in inode_ref_link_count:
            links = inode_ref_link_count[inode]

        if links != link_count:
            print('INODE ' + str(inode) + ' HAS ' + str(links) +
                  ' LINKS BUT LINKCOUNT IS ' + str(link_count))
            corrupted_flag = 1


def find_unallocated_directory():
    global inode_ref, free_inode, inode_to_parent, corrupted_flag
    # for key,value in inode_ref.items():
    #     print(key,value)
    for i, dirname in inode_ref.items():
        if i in free_inode and i in inode_to_parent:
            print('DIRECTORY INODE ' +
                  str(inode_to_parent[i]) + ' NAME ' + dirname + " UNALLOCATED INODE " + str(i))
            corrupted_flag = 1


def main():
    global corrupted_flag
    read_CSV()
    find_unreferenced()
    find_unallocated_freelist()
    find_duplicate_blocks()
    check_dir_link()
    check_link_count()
    find_unallocated_directory()

    if not corrupted_flag:
        sys.exit(0)
    sys.exit(2)


if __name__ == "__main__":
    main()
