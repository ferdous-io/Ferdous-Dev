/************************************************************
 * PharmaTrack - Pharmacy Inventory & Assistance System
 * Full working version with file I/O (binary)
 * Author: Team (Maruf, Ferdous, Pial, Nasif)
 * Compile: gcc pharmatrack.c -o pharmatrack
 ************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_MED 1000
#define NAME_LEN 80
#define TREAT_LEN 80
#define DATA_FILE "pharmatrack.dat"

typedef struct {
    char name[NAME_LEN];
    char treatment[TREAT_LEN];
    int quantity;
    float price;
    int day, month, year;  // expiry date
    int shelf, row;        // location
    int id;                // unique id
} Medicine;

/* Global storage (in-memory) */
Medicine meds[MAX_MED];
int count = 0;
int next_id = 1;

/* Function declarations */
void loadFromFile();
void saveToFile();

void addMedicine();
void displayAll();
int searchMedicineByName(const char *name);
void updateMedicine();
void deleteMedicine();
void checkExpiry();                       // show expired or expiring soon
void findAlternatives();                  // by treatment
void showShelfLocation();
void searchByTreatment();
void billing();                           // price * qty + update stock
int isExpiredOrSoon(int d, int m, int y, int *expiredFlag); // returns daysLeft or -1
void clear_stdin();
void pauseConsole();

/* ---------------------- main -------------------------- */
int main() {
    int choice;
    loadFromFile();

    while (1) {
        printf("\n====== PharmaTrack Menu ======\n");
        printf("1. Add Medicine\n");
        printf("2. Display All Medicines\n");
        printf("3. Search Medicine (by name)\n");
        printf("4. Update Medicine (by name)\n");
        printf("5. Delete Medicine (by name)\n");
        printf("6. Check Expiry Status\n");
        printf("7. Alternative Medicine Suggestion\n");
        printf("8. Show Shelf Location (by name)\n");
        printf("9. Search By Treatment\n");
        printf("10. Billing (sell medicine)\n");
        printf("11. Save & Exit\n");
        printf("Select an option: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Try again.\n");
            clear_stdin();
            continue;
        }
        clear_stdin();

        switch (choice) {
            case 1: addMedicine(); break;
            case 2: displayAll(); break;
            case 3: {
                char name[NAME_LEN];
                printf("Enter medicine name to search: ");
                fgets(name, NAME_LEN, stdin);
                name[strcspn(name, "\n")] = '\0';
                int idx = searchMedicineByName(name);
                if (idx == -1) printf("Medicine '%s' not found.\n", name);
                else {
                    Medicine *m = &meds[idx];
                    printf("Found (ID %d): %s | Qty: %d | Price: %.2f | Expiry: %02d-%02d-%04d | Treatment: %s | Shelf: %d Row: %d\n",
                           m->id, m->name, m->quantity, m->price, m->day, m->month, m->year, m->treatment, m->shelf, m->row);
                }
            } break;

            case 4: updateMedicine(); break;
            case 5: deleteMedicine(); break;
            case 6: checkExpiry(); break;
            case 7: findAlternatives(); break;
            case 8: showShelfLocation(); break;
            case 9: searchByTreatment(); break;
            case 10: billing(); break;
            case 11: saveToFile(); printf("Data saved. Exiting.\n"); return 0;
            default: printf("Invalid option.\n");
        }
    }
}

/* ------------------ utility ------------------- */

void clear_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void pauseConsole() {
    printf("Press ENTER to continue...");
    getchar();
}

/* ------------------ file handling ------------------- */

void loadFromFile() {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) {
        // No existing file; start fresh
        count = 0;
        next_id = 1;
        return;
    }
    if (fread(&count, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return;
    }
    if (count < 0 || count > MAX_MED) { fclose(fp); count = 0; return; }
    if (fread(&next_id, sizeof(int), 1, fp) != 1) { fclose(fp); return; }
    if (fread(meds, sizeof(Medicine), count, fp) != (size_t)count) {
        // partial; ignore
    }
    fclose(fp);
    printf("Loaded %d records from %s\n", count, DATA_FILE);
}

void saveToFile() {
    FILE *fp = fopen(DATA_FILE, "wb");
    if (!fp) {
        printf("Error: cannot open data file for writing.\n");
        return;
    }
    fwrite(&count, sizeof(int), 1, fp);
    fwrite(&next_id, sizeof(int), 1, fp);
    if (count > 0) fwrite(meds, sizeof(Medicine), count, fp);
    fclose(fp);
}

/* ------------------ core functions ------------------- */

void addMedicine() {
    if (count >= MAX_MED) {
        printf("Storage full. Cannot add more medicines.\n");
        return;
    }
    Medicine m;
    memset(&m, 0, sizeof(Medicine));

    printf("Enter medicine name: ");
    fgets(m.name, NAME_LEN, stdin);
    m.name[strcspn(m.name, "\n")] = '\0';

    printf("Enter treatment/category (e.g., fever, pain): ");
    fgets(m.treatment, TREAT_LEN, stdin);
    m.treatment[strcspn(m.treatment, "\n")] = '\0';

    printf("Enter quantity (integer): ");
    scanf("%d", &m.quantity);
    clear_stdin();

    printf("Enter price per unit (e.g., 12.50): ");
    scanf("%f", &m.price);
    clear_stdin();

    printf("Enter expiry date (DD MM YYYY): ");
    scanf("%d %d %d", &m.day, &m.month, &m.year);
    clear_stdin();

    printf("Enter shelf number (int): ");
    scanf("%d", &m.shelf);
    clear_stdin();

    printf("Enter row number (int): ");
    scanf("%d", &m.row);
    clear_stdin();

    m.id = next_id++;

    meds[count++] = m;
    printf("Medicine '%s' added with ID %d.\n", m.name, m.id);
}

void displayAll() {
    if (count == 0) {
        printf("No medicines in inventory.\n");
        return;
    }
    printf("\n%-4s | %-30s | %-10s | %-5s | %-8s | %-10s | %-6s\n",
           "ID", "Name", "Treatment", "Qty", "Price", "Expiry", "Shelf/Row");
    printf("-------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; ++i) {
        Medicine *m = &meds[i];
        printf("%-4d | %-30s | %-10s | %-5d | %-8.2f | %02d-%02d-%04d | %02d/%02d\n",
               m->id, m->name, m->treatment, m->quantity, m->price, m->day, m->month, m->year, m->shelf, m->row);
    }
    pauseConsole();
}

int searchMedicineByName(const char *name) {
    if (!name || strlen(name) == 0) return -1;
    for (int i = 0; i < count; ++i) {
        if (strcasecmp(meds[i].name, name) == 0) return i;
    }
    return -1;
}

void updateMedicine() {
    char name[NAME_LEN];
    printf("Enter medicine name to update: ");
    fgets(name, NAME_LEN, stdin);
    name[strcspn(name, "\n")] = '\0';
    int idx = searchMedicineByName(name);
    if (idx == -1) { printf("Medicine not found.\n"); return; }

    Medicine *m = &meds[idx];
    printf("Updating '%s' (ID %d)\n", m->name, m->id);
    printf("Leave blank or enter 0 to keep current value.\n");

    printf("New name (current: %s): ", m->name);
    char buf[NAME_LEN];
    fgets(buf, NAME_LEN, stdin);
    if (buf[0] != '\n') {
        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) > 0) strcpy(m->name, buf);
    }

    printf("New treatment (current: %s): ", m->treatment);
    fgets(buf, TREAT_LEN, stdin);
    if (buf[0] != '\n') {
        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) > 0) strcpy(m->treatment, buf);
    }

    printf("New quantity (current: %d): ", m->quantity);
    int v;
    if (scanf("%d", &v) == 1 && v > 0) m->quantity = v;
    clear_stdin();

    printf("New price (current: %.2f): ", m->price);
    float f;
    if (scanf("%f", &f) == 1 && f > 0) m->price = f;
    clear_stdin();

    printf("New expiry (DD MM YYYY) (current: %02d-%02d-%04d): ", m->day, m->month, m->year);
    int d, mo, y;
    if (scanf("%d %d %d", &d, &mo, &y) == 3) {
        if (d > 0 && mo > 0 && y > 0) { m->day = d; m->month = mo; m->year = y; }
    }
    clear_stdin();

    printf("New shelf (current: %d): ", m->shelf);
    if (scanf("%d", &v) == 1 && v >= 0) m->shelf = v;
    clear_stdin();

    printf("New row (current: %d): ", m->row);
    if (scanf("%d", &v) == 1 && v >= 0) m->row = v;
    clear_stdin();

    printf("Medicine updated.\n");
}

void deleteMedicine() {
    char name[NAME_LEN];
    printf("Enter medicine name to delete: ");
    fgets(name, NAME_LEN, stdin);
    name[strcspn(name, "\n")] = '\0';
    int idx = searchMedicineByName(name);
    if (idx == -1) { printf("Medicine not found.\n"); return; }

    printf("Are you sure you want to delete '%s' (ID %d)? (y/n): ", meds[idx].name, meds[idx].id);
    char c = getchar();
    clear_stdin();
    if (c == 'y' || c == 'Y') {
        for (int i = idx; i < count - 1; ++i) meds[i] = meds[i+1];
        count--;
        printf("Deleted.\n");
    } else printf("Delete cancelled.\n");
}

void checkExpiry() {
    if (count == 0) { printf("No medicines.\n"); return; }
    time_t t = time(NULL);
    struct tm now = *localtime(&t);
    int expiredFlag;
    int soonCount = 0, expiredCount = 0;

    printf("\nExpiry Report (Expired or expiring within 30 days):\n");
    printf("%-4s | %-30s | %-10s | %-8s | %-10s\n", "ID", "Name", "Qty", "Expiry", "Status");
    printf("-----------------------------------------------------------------\n");
    for (int i = 0; i < count; ++i) {
        int daysLeft = isExpiredOrSoon(meds[i].day, meds[i].month, meds[i].year, &expiredFlag);
        if (expiredFlag || (daysLeft >= 0 && daysLeft <= 30)) {
            if (expiredFlag) expiredCount++;
            else soonCount++;
            printf("%-4d | %-30s | %-10d | %02d-%02d-%04d | %s\n",
                   meds[i].id, meds[i].name, meds[i].quantity,
                   meds[i].day, meds[i].month, meds[i].year,
                   expiredFlag ? "EXPIRED" : "Expiring soon");
        }
    }
    printf("\nExpired: %d, Expiring within 30 days: %d\n", expiredCount, soonCount);
    pauseConsole();
}

/* returns -1 if past; if not past returns days left and expiredFlag=0; if expiredFlag=1 it's expired */
int isExpiredOrSoon(int d, int m, int y, int *expiredFlag) {
    time_t t = time(NULL);
    struct tm now = *localtime(&t);

    struct tm exp = {0};
    exp.tm_mday = d;
    exp.tm_mon = m - 1;
    exp.tm_year = y - 1900;
    exp.tm_hour = 0;
    exp.tm_min = 0;
    exp.tm_sec = 0;
    time_t texp = mktime(&exp);
    if (texp == (time_t)-1) {
        *expiredFlag = 0;
        return -2; // invalid date
    }

    double diff = difftime(texp, t); // seconds
    int days = (int)(diff / (60 * 60 * 24));
    if (days < 0) {
        *expiredFlag = 1;
        return -1;
    } else {
        *expiredFlag = 0;
        return days;
    }
}

void findAlternatives() {
    char name[NAME_LEN];
    printf("Enter medicine name to get alternatives: ");
    fgets(name, NAME_LEN, stdin);
    name[strcspn(name, "\n")] = '\0';
    int idx = searchMedicineByName(name);
    if (idx == -1) { printf("Medicine '%s' not found.\n", name); return; }

    char *treat = meds[idx].treatment;
    printf("Alternatives for '%s' (treatment: %s):\n", meds[idx].name, treat);
    int found = 0;
    for (int i = 0; i < count; ++i) {
        if (i == idx) continue;
        if (strcasecmp(meds[i].treatment, treat) == 0 && meds[i].quantity > 0) {
            printf("- %s (Qty: %d, Price: %.2f) [Shelf %d Row %d]\n", meds[i].name, meds[i].quantity, meds[i].price, meds[i].shelf, meds[i].row);
            found++;
        }
    }
    if (!found) printf("No alternatives found in same treatment category.\n");
    pauseConsole();
}

void showShelfLocation() {
    char name[NAME_LEN];
    printf("Enter medicine name to show shelf: ");
    fgets(name, NAME_LEN, stdin);
    name[strcspn(name, "\n")] = '\0';
    int idx = searchMedicineByName(name);
    if (idx == -1) { printf("Medicine not found.\n"); return; }
    Medicine *m = &meds[idx];
    printf("Medicine '%s' is located at Shelf %d, Row %d. Quantity: %d\n", m->name, m->shelf, m->row, m->quantity);
}

void searchByTreatment() {
    char treat[TREAT_LEN];
    printf("Enter treatment/category to search: ");
    fgets(treat, TREAT_LEN, stdin);
    treat[strcspn(treat, "\n")] = '\0';
    int found = 0;
    printf("\n%-4s | %-30s | %-5s | %-8s\n", "ID", "Name", "Qty", "Price");
    printf("---------------------------------------------------------\n");
    for (int i = 0; i < count; ++i) {
        if (strcasecmp(meds[i].treatment, treat) == 0) {
            printf("%-4d | %-30s | %-5d | %-8.2f\n", meds[i].id, meds[i].name, meds[i].quantity, meds[i].price);
            found++;
        }
    }
    if (!found) printf("No medicines found for treatment '%s'\n", treat);
    pauseConsole();
}

void billing() {
    char name[NAME_LEN];
    printf("Enter medicine name to sell: ");
    fgets(name, NAME_LEN, stdin);
    name[strcspn(name, "\n")] = '\0';
    int idx = searchMedicineByName(name);
    if (idx == -1) { printf("Medicine not found.\n"); return; }
    Medicine *m = &meds[idx];
    printf("Medicine: %s | Price: %.2f | Available qty: %d\n", m->name, m->price, m->quantity);
    printf("Enter quantity to sell: ");
    int q;
    if (scanf("%d", &q) != 1) { clear_stdin(); printf("Invalid number.\n"); return; }
    clear_stdin();
    if (q <= 0) { printf("Quantity must be positive.\n"); return; }
    if (q > m->quantity) { printf("Not enough stock. Available: %d\n", m->quantity); return; }

    float total = q * m->price;
    m->quantity -= q;
    printf("Total: %.2f\n", total);
    printf("Proceed with sale? (y/n): ");
    char c = getchar(); clear_stdin();
    if (c == 'y' || c == 'Y') {
        printf("Sale completed. Remaining qty: %d\n", m->quantity);
    } else {
        // rollback
        m->quantity += q;
        printf("Sale cancelled.\n");
    }
}

/* End of file */
