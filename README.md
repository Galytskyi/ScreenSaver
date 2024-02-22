-------------------------------------------------------------------

Qt 6.5.3
Qt Creator 12.0.2
MinGW 11.2.0 x64

-------------------------------------------------------------------

Description

The program should take a screenshot every minute (start/stop should be done with a button). Each
new snapshot should be compared with the previous snapshot and output the result in % how
similar the two snapshots (newly created and previous) are to each other.

Comparison of the snapshot with the previous one should be carried out in a separate thread and
not block the main thread, and after comparing and processing the snapshot, it should be displayed
on the grid (Grid View) with an indicator of similarity to the previous snapshot.

The snapshots, hash sum and % should be stored in the DB, and when the program is restarted,
these snapshots should be loaded into the program.

-------------------------------------------------------------------
