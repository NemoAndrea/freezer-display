import sys
import zipfile
from pathlib import Path

# call with `python pre-commit-FCStd.py <YOURFILE>[.FCStd]`

if len(sys.argv) < 2:
    print("Error: Please provide a path to a FreeCAD file.")
    sys.exit(1)

fc_file = Path(sys.argv[1])

if not fc_file.is_file():
    print(f"Error: '{fc_file}' is not a valid file.")
    sys.exit(1)


### Thumbnail Extraction -------------------------------------------------------

# Paths for thumbnail handling and temporary zip writing
thumb_dir = Path("thumbnails")
thumb_file = thumb_dir / "Thumbnail.png"
temp_file = fc_file.with_name(f"{fc_file.stem}_temp{fc_file.suffix}")

# We try to extract a thumbnail image from the FreeCAD file. It may not exist,
# as the user may have disabled the saving of Thumbnails in FreeCAD, or may 
# have reverted a commit, resulting in an unstaged FreeCAD file with the thumbnail
# already extracted.
try:
    with zipfile.ZipFile(fc_file, "r") as zip_read:
        zip_read.extract("thumbnails/Thumbnail.png")
        
        with zipfile.ZipFile(temp_file, "w", compression=zipfile.ZIP_STORED) as zip_write:
            for item in zip_read.infolist():
                if item.filename != "thumbnails/Thumbnail.png":
                    zip_write.writestr(
                        item,
                        zip_read.read(item.filename),
                        compress_type=zipfile.ZIP_STORED  # no compression
                    )

    # Move extracted thumbnail and remove the old 'thumbnails/' directory
    thumbnail_path = Path(fc_file.parent / ".thumbnails" / f"{fc_file.stem}.png")
    thumbnail_path.parent.mkdir(exist_ok=True)
    thumb_file.replace(thumbnail_path)
    thumb_dir.rmdir()  # remove the old zip directory

    # Replace the original FreeCAD file with the thumbnail-less version
    temp_file.replace(fc_file)

    # Report on action
    filesize_kb = thumbnail_path.stat().st_size / 1024
    print(f"+ Extracted thumbnail image ({filesize_kb:.1f}kb)")

except KeyError:
    print(
        f"FreeCAD file '{fc_file.name}' does not contain a thumbnail. "
        "Check your FreeCAD configuration to make sure that a thumbnail "
        "is automatically included with the save file."
    )


### BOM Extraction -------------------------------------------------------------

import xml.etree.ElementTree as ET
import csv
import re
from collections import defaultdict

# Extract the internal BOM into a CSV file and remove the content from the
# FreeCAD file to reduce disk usage. It will be recomputed in FreeCAD.

# TODO: consider whether it would be beneficial to mark the BOM for recompute

with zipfile.ZipFile(fc_file, "r") as zip_read:

    temp_file = fc_file.with_name(f"{fc_file.stem}_temp{fc_file.suffix}")

    with zip_read.open("Document.xml") as f:
        xml_content = f.read().decode("utf-8")

        root = ET.fromstring(xml_content)

# try to find the XML element that describes the bom, which sits under
# Document --> ObjectData --> <Object name="Bill_of_Materials> --> Properties --> <Property name='cells'> --> Cells
bom = root.find("ObjectData/Object[@name='Bill_of_Materials']/Properties/Property[@name='cells']/Cells")

# look for BOM match. If no BOM is found (which may well be the case), then
# do not print anything as to keep the commit hook message clean..
if bom is not None:
    #print(ET.tostring(bom, encoding="utf-8").decode("utf-8"))
    cells = bom.findall("Cell")

    # if there is at least one BOM element, extract a CSV
    if cells: 
        grid = defaultdict(dict)
        columns = set()
        for cell in cells:
            address = cell.get("address").strip()
            content = cell.get("content", "")  # default to ""
            if content.startswith("'"):  # for some reason content has leading '
                content = content[1:].strip()
            # custom fields start with "." in FreeCAD, but this does not need 
            # to be included in the csv file
            if content.startswith("."):  
                content = content[1:].strip()

            # match column index (letter A,B,C...) and row index (1,2,3,...)
            match = re.match(r"([A-Z]+)(\d+)", address)
            if match:
                col, row = match.group(1), int(match.group(2))
                grid[row][col] = content
                columns.add(col)

            bom.remove(cell)  # remove the cell from XML object
        bom.set("Count", "0")  # also update the Count attribute


        header = list(grid[1].values())
        row_numbers = list(grid.keys())  # could be done with len() instead
        cols_sorted = sorted(columns)  # ensure set is ordered
        rows = []
        for row_idx in row_numbers[1:]:
            # make a list by looking for value at [A], [B] etc. Add "" if not present
            rows.append([grid[row_idx].get(col, "") for col in cols_sorted])

        # write BOM csv 
        CSV_path = fc_file.parent / f"BOM_{fc_file.stem}.csv"
        with open(Path(CSV_path), mode="w", newline="", encoding="utf-8") as file:
            writer = csv.writer(file)
            writer.writerow(header)  
            writer.writerows(rows)  

        # Re-write the FreeCAD file, removing the BOM entries from
        # Document.xml. They will be recomputed when the BOM is opened again.
        with zipfile.ZipFile(fc_file, "r") as zip_read:
            with zipfile.ZipFile(temp_file, "w", compression=zipfile.ZIP_STORED) as zip_write:
                for item in zip_read.infolist():
                    if item.filename != "Document.xml":
                        zip_write.writestr(
                            item,
                            zip_read.read(item.filename),
                            compress_type=zipfile.ZIP_STORED  # no compression
                        )
                    else:
                        stripped_XML = ET.tostring(root, encoding="unicode", xml_declaration=True)
                        zip_write.writestr(
                            "Document.xml",
                            stripped_XML,
                            compress_type=zipfile.ZIP_STORED  # no compression
                        )
        temp_file.replace(fc_file)
        
        # Extraction complete
        filesize_kb = CSV_path.stat().st_size / 1024
        print(f"+ Extracted BOM ({len(rows)} entries, {filesize_kb:.1f}kb)")

    else: 
        # If we previously extracted BOM entries, then the XML element
        # will exist, but no cell elements will exist.
        # this should be rare (only occuring after `git reset`)
        print("- BOM found, but no BOM entries are present")

    

