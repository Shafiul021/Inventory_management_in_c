#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME_LEN 50
#define LOW_STOCK_THRESHOLD 10
#define MAX_PRODUCTS 1000
#define MAX_TRANSACTIONS 5000

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    float price;
    int quantity;
} Product;

typedef struct {
    int transaction_id;
    char date_time[20]; // Format: YYYY-MM-DD_HH:MM
    int product_id;
    char type; // 'S' for Sale, 'P' for Purchase
    int quantity;
    float total_amount;
} Transaction;

// Global Data
Product inventory[MAX_PRODUCTS];
int product_count = 0;

Transaction sales_log[MAX_TRANSACTIONS];
int transaction_count = 0;

// Function Prototypes
void loadData();
void saveData();
void adminLogin();
void mainMenu();

void addProduct();
void viewProducts();
void searchProduct();
void updateProduct();
void deleteProduct();

void recordPurchase();
void recordSale();

void checkLowStockAlerts();
void viewSalesHistory();
void calculateInventoryValue();
void generateDailyReport();

// Utility function to get current time string
void getCurrentTime(char* buffer) {
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 20, "%Y-%m-%d_%H:%M", timeinfo);
}

// Utility function to get current date string
void getCurrentDate(char* buffer) {
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 20, "%Y-%m-%d", timeinfo);
}

int findProductIndex(int id) {
    for (int i = 0; i < product_count; i++) {
        if (inventory[i].id == id) {
            return i;
        }
    }
    return -1;
}

int main() {
    loadData();
    adminLogin();
    mainMenu();
    return 0;
}

void loadData() {
    FILE *f1 = fopen("inventory.txt", "r");
    if (f1 != NULL) {
        while (fscanf(f1, "%d %49s %f %d",
                      &inventory[product_count].id,
                      inventory[product_count].name,
                      &inventory[product_count].price,
                      &inventory[product_count].quantity) == 4) {
            product_count++;
        }
        fclose(f1);
    }

    FILE *f2 = fopen("sales_log.txt", "r");
    if (f2 != NULL) {
        while (fscanf(f2, "%d %19s %d %c %d %f",
                      &sales_log[transaction_count].transaction_id,
                      sales_log[transaction_count].date_time,
                      &sales_log[transaction_count].product_id,
                      &sales_log[transaction_count].type,
                      &sales_log[transaction_count].quantity,
                      &sales_log[transaction_count].total_amount) == 6) {
            transaction_count++;
        }
        fclose(f2);
    }
}

void saveData() {
    FILE *f1 = fopen("inventory.txt", "w");
    if (f1 != NULL) {
        for (int i = 0; i < product_count; i++) {
            fprintf(f1, "%d %s %.2f %d\n",
                    inventory[i].id,
                    inventory[i].name,
                    inventory[i].price,
                    inventory[i].quantity);
        }
        fclose(f1);
    } else {
        printf("Error saving inventory data.\n");
    }

    FILE *f2 = fopen("sales_log.txt", "w");
    if (f2 != NULL) {
        for (int i = 0; i < transaction_count; i++) {
            fprintf(f2, "%d %s %d %c %d %.2f\n",
                    sales_log[i].transaction_id,
                    sales_log[i].date_time,
                    sales_log[i].product_id,
                    sales_log[i].type,
                    sales_log[i].quantity,
                    sales_log[i].total_amount);
        }
        fclose(f2);
    } else {
        printf("Error saving sales log.\n");
    }
}

void adminLogin() {
    char username[20];
    char password[20];
    printf("==================================\n");
    printf("    Inventory Management System   \n");
    printf("==================================\n");

    while (1) {
        printf("Admin Username: ");
        scanf("%19s", username);
        printf("Password: ");
        scanf("%19s", password);

        if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
            printf("Login successful!\n\n");
            break;
        } else {
            printf("Invalid credentials. Try again.\n");
        }
    }
}

void mainMenu() {
    int choice;
    do {
        printf("\n================ Main Menu ================\n");
        printf("1. Add New Product\n");
        printf("2. View All Products\n");
        printf("3. Search Product\n");
        printf("4. Update Product Details\n");
        printf("5. Delete Product\n");
        printf("6. Record Stock Purchase\n");
        printf("7. Record Sale\n");
        printf("8. Low-Stock Alerts\n");
        printf("9. View Sales History\n");
        printf("10. Calculate Total Inventory Value\n");
        printf("11. Generate Daily Sales Report\n");
        printf("12. Save & Exit\n");
        printf("Enter your choice: ");

        if(scanf("%d", &choice) != 1) {
            while(getchar() != '\n'); // clear buffer
            continue;
        }

        switch (choice) {
            case 1: addProduct(); break;
            case 2: viewProducts(); break;
            case 3: searchProduct(); break;
            case 4: updateProduct(); break;
            case 5: deleteProduct(); break;
            case 6: recordPurchase(); break;
            case 7: recordSale(); break;
            case 8: checkLowStockAlerts(); break;
            case 9: viewSalesHistory(); break;
            case 10: calculateInventoryValue(); break;
            case 11: generateDailyReport(); break;
            case 12: saveData(); printf("Data saved. Exiting...\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 12);
}

void addProduct() {
    if (product_count >= MAX_PRODUCTS) {
        printf("Inventory is full!\n");
        return;
    }
    Product p;
    printf("Enter Product ID: ");
    scanf("%d", &p.id);
    if (findProductIndex(p.id) != -1) {
        printf("Product with ID %d already exists.\n", p.id);
        return;
    }
    printf("Enter Product Name (no spaces): ");
    scanf("%49s", p.name);
    printf("Enter Price: ");
    scanf("%f", &p.price);
    printf("Enter Quantity: ");
    scanf("%d", &p.quantity);

    inventory[product_count] = p;
    product_count++;
    printf("Product added successfully!\n");
}

void viewProducts() {
    printf("\nID\tName\t\tPrice\tQuantity\n");
    printf("----------------------------------------\n");
    for (int i = 0; i < product_count; i++) {
        printf("%d\t%-15s\t%.2f\t%d\n", inventory[i].id, inventory[i].name, inventory[i].price, inventory[i].quantity);
    }
}

void searchProduct() {
    int id;
    printf("Enter Product ID to search: ");
    scanf("%d", &id);
    int idx = findProductIndex(id);
    if (idx != -1) {
        printf("Found: ID: %d, Name: %s, Price: %.2f, Stock: %d\n",
               inventory[idx].id, inventory[idx].name, inventory[idx].price, inventory[idx].quantity);
    } else {
        printf("Product not found.\n");
    }
}

void updateProduct() {
    int id;
    printf("Enter Product ID to update: ");
    scanf("%d", &id);
    int idx = findProductIndex(id);
    if (idx != -1) {
        printf("Enter new Price (current: %.2f): ", inventory[idx].price);
        scanf("%f", &inventory[idx].price);
        printf("Enter new Quantity (current: %d): ", inventory[idx].quantity);
        scanf("%d", &inventory[idx].quantity);
        printf("Product updated.\n");
    } else {
        printf("Product not found.\n");
    }
}

void deleteProduct() {
    int id;
    printf("Enter Product ID to delete: ");
    scanf("%d", &id);
    int idx = findProductIndex(id);
    if (idx != -1) {
        for (int i = idx; i < product_count - 1; i++) {
            inventory[i] = inventory[i + 1];
        }
        product_count--;
        printf("Product deleted.\n");
    } else {
        printf("Product not found.\n");
    }
}

void recordPurchase() {
    int id, qty;
    printf("Enter Product ID for purchase: ");
    scanf("%d", &id);
    int idx = findProductIndex(id);
    if (idx == -1) {
        printf("Product not found.\n");
        return;
    }
    printf("Enter quantity purchased: ");
    scanf("%d", &qty);
    if (qty <= 0) return;

    inventory[idx].quantity += qty;

    Transaction t;
    t.transaction_id = transaction_count + 1;
    getCurrentTime(t.date_time);
    t.product_id = id;
    t.type = 'P';
    t.quantity = qty;
    t.total_amount = qty * inventory[idx].price; // assuming purchase cost is same as price for simplicity, or could ask for purchase cost

    sales_log[transaction_count++] = t;
    printf("Purchase recorded successfully.\n");
}

void recordSale() {
    int id, qty;
    printf("Enter Product ID for sale: ");
    scanf("%d", &id);
    int idx = findProductIndex(id);
    if (idx == -1) {
        printf("Product not found.\n");
        return;
    }
    printf("Enter quantity sold: ");
    scanf("%d", &qty);
    if (qty <= 0) return;

    if (inventory[idx].quantity < qty) {
        printf("Insufficient stock! Available: %d\n", inventory[idx].quantity);
        return;
    }

    inventory[idx].quantity -= qty;

    Transaction t;
    t.transaction_id = transaction_count + 1;
    getCurrentTime(t.date_time);
    t.product_id = id;
    t.type = 'S';
    t.quantity = qty;
    t.total_amount = qty * inventory[idx].price;

    sales_log[transaction_count++] = t;
    printf("Sale recorded! Total amount: %.2f\n", t.total_amount);
}

void checkLowStockAlerts() {
    printf("\n--- Low Stock Alerts ---\n");
    int found = 0;
    for (int i = 0; i < product_count; i++) {
        if (inventory[i].quantity <= LOW_STOCK_THRESHOLD) {
            printf("ALERT: %s (ID: %d) has low stock: %d\n", inventory[i].name, inventory[i].id, inventory[i].quantity);
            found = 1;
        }
    }
    if (!found) {
        printf("All products have sufficient stock.\n");
    }
}

void viewSalesHistory() {
    printf("\nTXN_ID\tDate_Time\t\tProdID\tType\tQty\tAmount\n");
    printf("------------------------------------------------------------------\n");
    for (int i = 0; i < transaction_count; i++) {
        printf("%d\t%s\t%d\t%c\t%d\t%.2f\n",
               sales_log[i].transaction_id,
               sales_log[i].date_time,
               sales_log[i].product_id,
               sales_log[i].type,
               sales_log[i].quantity,
               sales_log[i].total_amount);
    }
}

void calculateInventoryValue() {
    float total = 0.0;
    for (int i = 0; i < product_count; i++) {
        total += (inventory[i].price * inventory[i].quantity);
    }
    printf("Total Inventory Value: %.2f\n", total);
}

void generateDailyReport() {
    char today[20];
    getCurrentDate(today);

    printf("\n--- Daily Sales Report: %s ---\n", today);
    float total_sales = 0;
    int total_items_sold = 0;

    for (int i = 0; i < transaction_count; i++) {
        if (strncmp(sales_log[i].date_time, today, 10) == 0 && sales_log[i].type == 'S') {
            total_sales += sales_log[i].total_amount;
            total_items_sold += sales_log[i].quantity;
        }
    }

    printf("Total Items Sold: %d\n", total_items_sold);
    printf("Total Revenue: %.2f\n", total_sales);
}
