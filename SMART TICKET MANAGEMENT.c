#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HEAP_SIZE 100
#define HASH_TABLE_SIZE 23
#define MAX_STR 100

// ==========================================
// 1. DATA STRUCTURE DEFINITIONS
// ==========================================

typedef struct Ticket {
    int id;
    char customerName[MAX_STR];
    char issue[MAX_STR];
    int priority; // 1 = Critical, 2 = High, 3 = Medium, 4 = Low, 5 = Trivial
    char status[20]; // "OPEN" or "RESOLVED"
} Ticket;

// --- Hash Table Node (Separate Chaining for O(1) ID Lookup) ---
typedef struct HashNode {
    Ticket data;
    struct HashNode* next;
} HashNode;

// --- Priority Queue (Min-Heap based on Priority value) ---
typedef struct {
    Ticket heap[MAX_HEAP_SIZE];
    int size;
} PriorityQueue;

// --- Stack Node (Linked List for Resolved History & Undo) ---
typedef struct StackNode {
    Ticket data;
    struct StackNode* next;
} StackNode;

// Global DS instances
PriorityQueue pq;
HashNode* hashTable[HASH_TABLE_SIZE];
StackNode* resolvedStack = NULL;
int nextTicketId = 1001;

// ==========================================
// 2. HASH TABLE OPERATIONS (O(1) Search)
// ==========================================

int hashFunction(int id) {
    return id % HASH_TABLE_SIZE;
}

void insertHashTable(Ticket t) {
    int index = hashFunction(t.id);
    HashNode* newNode = (HashNode*)malloc(sizeof(HashNode));
    newNode->data = t;
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

Ticket* searchHashTable(int id) {
    int index = hashFunction(id);
    HashNode* temp = hashTable[index];
    while (temp != NULL) {
        if (temp->data.id == id) {
            return &(temp->data);
        }
        temp = temp->next;
    }
    return NULL;
}

void updateHashStatus(int id, const char* newStatus) {
    Ticket* t = searchHashTable(id);
    if (t != NULL) {
        strcpy(t->status, newStatus);
    }
}

// ==========================================
// 3. PRIORITY QUEUE / MIN-HEAP OPERATIONS
// ==========================================

void swap(Ticket* a, Ticket* b) {
    Ticket temp = *a;
    *a = *b;
    *b = temp;
}

void heapifyUp(int index) {
    int parent = (index - 1) / 2;
    // Lower number = Higher urgency. Tie-breaker: lower Ticket ID first.
    while (index > 0 && 
          (pq.heap[index].priority < pq.heap[parent].priority ||
          (pq.heap[index].priority == pq.heap[parent].priority && pq.heap[index].id < pq.heap[parent].id))) {
        swap(&pq.heap[index], &pq.heap[parent]);
        index = parent;
        parent = (index - 1) / 2;
    }
}

void heapifyDown(int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < pq.size && 
       (pq.heap[left].priority < pq.heap[smallest].priority ||
       (pq.heap[left].priority == pq.heap[smallest].priority && pq.heap[left].id < pq.heap[smallest].id))) {
        smallest = left;
    }

    if (right < pq.size && 
       (pq.heap[right].priority < pq.heap[smallest].priority ||
       (pq.heap[right].priority == pq.heap[smallest].priority && pq.heap[right].id < pq.heap[smallest].id))) {
        smallest = right;
    }

    if (smallest != index) {
        swap(&pq.heap[index], &pq.heap[smallest]);
        heapifyDown(smallest);
    }
}

void enqueueTicket(Ticket t) {
    if (pq.size >= MAX_HEAP_SIZE) {
        printf("\n[!] Error: Ticket Queue is full!\n");
        return;
    }
    pq.heap[pq.size] = t;
    heapifyUp(pq.size);
    pq.size++;
}

Ticket dequeueTicket() {
    Ticket top = pq.heap[0];
    pq.heap[0] = pq.heap[pq.size - 1];
    pq.size--;
    heapifyDown(0);
    return top;
}

// ==========================================
// 4. STACK OPERATIONS (Resolved History/Undo)
// ==========================================

void pushStack(Ticket t) {
    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
    newNode->data = t;
    newNode->next = resolvedStack;
    resolvedStack = newNode;
}

int popStack(Ticket* poppedTicket) {
    if (resolvedStack == NULL) return 0;
    StackNode* temp = resolvedStack;
    *poppedTicket = temp->data;
    resolvedStack = resolvedStack->next;
    free(temp);
    return 1;
}

// ==========================================
// 5. CORE APPLICATION FEATURES
// ==========================================

const char* getPriorityLabel(int p) {
    switch (p) {
        case 1: return "1-CRITICAL";
        case 2: return "2-HIGH";
        case 3: return "3-MEDIUM";
        case 4: return "4-LOW";
        default: return "5-TRIVIAL";
    }
}

void createNewTicket() {
    Ticket t;
    t.id = nextTicketId++;
    strcpy(t.status, "OPEN");

    printf("\n--- Create New Support Ticket ---\n");
    printf("Enter Customer Name: ");
    fgets(t.customerName, MAX_STR, stdin);
    t.customerName[strcspn(t.customerName, "\n")] = 0;

    printf("Enter Issue Description: ");
    fgets(t.issue, MAX_STR, stdin);
    t.issue[strcspn(t.issue, "\n")] = 0;

    do {
        printf("Select Priority (1:Critical, 2:High, 3:Medium, 4:Low, 5:Trivial): ");
        scanf("%d", &t.priority);
        getchar(); // Clear newline buffer
    } while (t.priority < 1 || t.priority > 5);

    enqueueTicket(t);
    insertHashTable(t);

    printf("\n[+] Ticket Created Successfully! Assigned ID: #%d\n", t.id);
}

void processNextTicket() {
    if (pq.size == 0) {
        printf("\n[i] No pending open tickets in the Priority Queue!\n");
        return;
    }

    Ticket resolved = dequeueTicket();
    strcpy(resolved.status, "RESOLVED");
    updateHashStatus(resolved.id, "RESOLVED");
    pushStack(resolved);

    printf("\n==================================================\n");
    printf("   RESOLVED HIGHEST-PRIORITY TICKET (MIN-HEAP)    \n");
    printf("==================================================\n");
    printf(" Ticket ID : #%d\n", resolved.id);
    printf(" Customer  : %s\n", resolved.customerName);
    printf(" Priority  : %s\n", getPriorityLabel(resolved.priority));
    printf(" Issue     : %s\n", resolved.issue);
    printf(" Status    : %s\n", resolved.status);
    printf("==================================================\n");
}

void undoLastResolution() {
    Ticket reopened;
    if (!popStack(&reopened)) {
        printf("\n[!] Undo Stack is empty! No resolved tickets to reopen.\n");
        return;
    }

    strcpy(reopened.status, "OPEN");
    updateHashStatus(reopened.id, "OPEN");
    enqueueTicket(reopened);

    printf("\n[?] Undo Successful! Ticket #%d (%s) has been REOPENED and returned to the queue.\n",
           reopened.id, reopened.customerName);
}

void searchTicketByID() {
    int id;
    printf("\nEnter Ticket ID to search: ");
    scanf("%d", &id);
    getchar();

    Ticket* found = searchHashTable(id);
    if (found == NULL) {
        printf("\n[!] Ticket #%d not found in the Hash Table.\n", id);
    } else {
        printf("\n--- Ticket Details (O(1) Hash Lookup) ---\n");
        printf(" ID       : #%d\n", found->id);
        printf(" Customer : %s\n", found->customerName);
        printf(" Priority : %s\n", getPriorityLabel(found->priority));
        printf(" Issue    : %s\n", found->issue);
        printf(" Status   : %s\n", found->status);
        printf("-----------------------------------------\n");
    }
}

void displayActiveQueue() {
	int i;
    if (pq.size == 0) {
        printf("\n[i] Active queue is currently empty.\n");
        return;
    }

    printf("\n+--------+------------------+--------------+-------------------------------+----------+\n");
    printf("| ID     | Customer Name    | Priority     | Issue Description             | Status   |\n");
    printf("+--------+------------------+--------------+-------------------------------+----------+\n");
    for (i = 0; i < pq.size; i++) {
        printf("| #%-5d | %-16.16s | %-12s | %-29.29s | %-8s |\n",
               pq.heap[i].id,
               pq.heap[i].customerName,
               getPriorityLabel(pq.heap[i].priority),
               pq.heap[i].issue,
               pq.heap[i].status);
    }
    printf("+--------+------------------+--------------+-------------------------------+----------+\n");
    printf("* Note: Top row (#%d) is the root of the Min-Heap and will be processed next.\n", pq.heap[0].id);
}

void displayResolvedHistory() {
    if (resolvedStack == NULL) {
        printf("\n[i] No tickets have been resolved yet.\n");
        return;
    }

    printf("\n--- Resolved Ticket History (LIFO Stack) ---\n");
    StackNode* temp = resolvedStack;
    while (temp != NULL) {
        printf("[#%d] %s | %s | Issue: %s\n",
               temp->data.id,
               getPriorityLabel(temp->data.priority),
               temp->data.customerName,
               temp->data.issue);
        temp = temp->next;
    }
}

void exportToCSV() {
	int i;
    FILE* fp = fopen("tickets_report.csv", "w");
    if (!fp) {
        printf("\n[!] Failed to create file.\n");
        return;
    }

    fprintf(fp, "TicketID,CustomerName,Priority,Issue,Status\n");
    for (i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode* temp = hashTable[i];
        while (temp != NULL) {
            fprintf(fp, "%d,\"%s\",%s,\"%s\",%s\n",
                    temp->data.id,
                    temp->data.customerName,
                    getPriorityLabel(temp->data.priority),
                    temp->data.issue,
                    temp->data.status);
            temp = temp->next;
        }
    }
    fclose(fp);
    printf("\n[+] All tickets exported to 'tickets_report.csv' successfully!\n");
}

void seedDemoData() {
	int i;
    Ticket demo[] = {
        {nextTicketId++, "Aarav Sharma", "Payment gateway timeout on checkout", 2, "OPEN"},
        {nextTicketId++, "Priya Nair", "Typo on terms & conditions page", 5, "OPEN"},
        {nextTicketId++, "Rohan Das", "Production server database crash", 1, "OPEN"},
        {nextTicketId++, "Sneha Gupta", "Unable to reset account password", 3, "OPEN"}
    };

    for (i = 0; i < 4; i++) {
        enqueueTicket(demo[i]);
        insertHashTable(demo[i]);
    }
    printf("\n[+] Loaded 4 sample tickets into Min-Heap and Hash Table!\n");
}

// ==========================================
// 6. MAIN DRIVER MENU
// ==========================================

int main() {
	int i;
    pq.size = 0;
    for (i = 0; i < HASH_TABLE_SIZE; i++) hashTable[i] = NULL;

    int choice;
    printf("====================================================\n");
    printf("   SMART TICKET MANAGEMENT SYSTEM (C DATA STRUCTS)  \n");
    printf("   [Min-Heap PQ | Hash Table | LIFO Stack | CSV]    \n");
    printf("====================================================\n");

    seedDemoData(); // Pre-loads 4 sample tickets for quick testing

    while (1) {
        printf("\n-------------------- MAIN MENU ---------------------\n");
        printf(" 1. Create New Ticket          (Insert Heap + Hash)\n");
        printf(" 2. Process Next Urgent Ticket (Extract-Min + Push)\n");
        printf(" 3. View Pending Queue         (Display Min-Heap)\n");
        printf(" 4. Search Ticket by ID        (O(1) Hash Lookup)\n");
        printf(" 5. View Resolved History      (Traverse Stack)\n");
        printf(" 6. Undo Last Resolution       (Pop Stack + Re-Heap)\n");
        printf(" 7. Export All Tickets to CSV  (File I/O)\n");
        printf(" 0. Exit Program\n");
        printf("----------------------------------------------------\n");
        printf("Enter choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Handle non-integer input safely
            continue;
        }
        getchar(); // Consume newline

        switch (choice) {
            case 1: createNewTicket(); break;
            case 2: processNextTicket(); break;
            case 3: displayActiveQueue(); break;
            case 4: searchTicketByID(); break;
            case 5: displayResolvedHistory(); break;
            case 6: undoLastResolution(); break;
            case 7: exportToCSV(); break;
            case 0:
                printf("\nExiting Ticket Management System. Goodbye!\n");
                return 0;
            default:
                printf("\n[!] Invalid option. Please enter 0-7.\n");
        }
    }
}
