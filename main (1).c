/*
 * CLOUDSYNC STUDIO - Kiem toan & tai sap xep mang tai khoan dang ky (UserAccount)
 * Chuan C11. Bien dich: gcc -std=c11 -Wall -o main main.c
 *
 * RANG BUOC: khong ham tu dinh nghia - toan bo logic trong main().
 *
 * Goi:  0 Free      0 VND      toi da 1 thiet bi
 *       1 Standard  120,000    toi da 2 thiet bi
 *       2 Premium   300,000    toi da 5 thiet bi
 *
 * Thu tu kiem toan moi tai khoan:
 *   (1) active_devices <= 0              -> gan = 1                       (Loi du lieu 3)
 *   (2) remaining_days <= 0              -> ha cap Free, fee = 0, days = 0 (Quy tac het han, Loi 1)
 *       nguoc lai: devices > gioi han goi -> cat ve gioi han              (Quy tac vuot thiet bi)
 *   (3) chuan hoa monthly_fee theo plan_type SAU ha cap                  (Loi du lieu 2)
 * Sap xep (GIAI PHAP A - tai cho): Insertion Sort on dinh
 *   khoa: nhom tra phi (plan > 0) truoc, trong nhom tra phi fee giam dan, bang nhau giu thu tu goc.
 */
#include <stdio.h>
#include <string.h>

struct UserAccount {
    /* --- Thong tin nghiep vu (theo dac ta) --- */
    int account_id;
    int plan_type;          /* 0: Free, 1: Standard, 2: Premium */
    int monthly_fee;        /* VND */
    int remaining_days;
    int active_devices;
    /* --- Vet kiem toan (audit trail) - di chuyen cung ban ghi khi sap xep --- */
    int orig_plan;
    int orig_fee;
    int orig_days;
    int orig_devices;
    int flag_expired;       /* bi ha cap do het han */
    int flag_fee_fixed;     /* phi bi chuan hoa */
    int flag_dev_garbage;   /* so thiet bi rac <= 0 -> 1 */
    int flag_dev_capped;    /* bi cat giam thiet bi */
};

int main(void) {
    /* ---------------- Hang so nghiep vu ---------------- */
    const int MAX_N = 100;
    const int PHI_FREE = 0,  PHI_STANDARD = 120000, PHI_PREMIUM = 300000;
    const int TB_FREE = 1,   TB_STANDARD = 2,       TB_PREMIUM = 5;

    /* Bo du lieu mau bao phu du 4 loai loi du lieu */
    int demoId[10]      = { 1001,   1002,   1003, 1004,   1005, 1006,   1007,   1008,  1009,   1010 };
    int demoPlan[10]    = {    1,      2,      1,    2,      0,    2,      1,      0,     1,      2 };
    int demoFee[10]     = {120000, 300000,     0, 300000,    0, 250000, 120000, 50000, 150000, 300000 };
    int demoDays[10]    = {   30,     15,     20,   -5,     10,    7,      0,     30,    12,     25 };
    int demoDevices[10] = {    2,     99,      1,    3,      0,   -3,      4,      3,     5,      5 };

    struct UserAccount users[100];
    struct UserAccount temp;            /* bien tam duy nhat cho sap xep tai cho */

    int  n = 0, i, j, cheDo, nhapOK, hopLe, trungMa, gioiHan;
    int  truocNhau;                     /* 1: temp phai dung truoc users[j] */
    long long mrrTruoc = 0, mrrSau = 0;
    int  soHaCap = 0, soChuanHoaPhi = 0, soTbRac = 0, soCatTb = 0, soTraPhi = 0;

    printf("================ CLOUDSYNC STUDIO - SUBSCRIPTION AUDIT ================\n");

    /* ======================= BUOC 1: NAP DU LIEU ======================= */
    do {
        printf("Chon nguon du lieu (1: Nhap tay, 2: Bo du lieu mau 10 tai khoan): ");
        nhapOK = scanf("%d", &cheDo);
        while (getchar() != '\n');
        hopLe = (nhapOK == 1 && (cheDo == 1 || cheDo == 2));
        if (!hopLe) {
            printf("LOI: Chi chon 1 hoac 2.\n");
        }
    } while (!hopLe);

    if (cheDo == 2) {
        n = 10;
        for (i = 0; i < n; i++) {
            users[i].account_id     = demoId[i];
            users[i].plan_type      = demoPlan[i];
            users[i].monthly_fee    = demoFee[i];
            users[i].remaining_days = demoDays[i];
            users[i].active_devices = demoDevices[i];
        }
    } else {
        do {
            printf("Nhap so luong tai khoan N (1 - %d): ", MAX_N);
            nhapOK = scanf("%d", &n);
            while (getchar() != '\n');
            hopLe = (nhapOK == 1 && n >= 1 && n <= MAX_N);
            if (!hopLe) {
                printf("LOI: N phai la so nguyen trong khoang [1, %d]. Vui long nhap lai!\n", MAX_N);
            }
        } while (!hopLe);

        for (i = 0; i < n; i++) {
            printf("\n--- Tai khoan %d/%d ---\n", i + 1, n);

            do {
                printf("account_id (> 0): ");
                nhapOK = scanf("%d", &users[i].account_id);
                while (getchar() != '\n');
                hopLe = (nhapOK == 1 && users[i].account_id > 0);
                trungMa = 0;
                for (j = 0; hopLe && j < i; j++) {
                    if (users[j].account_id == users[i].account_id) {
                        trungMa = 1;
                    }
                }
                if (!hopLe) {
                    printf("LOI: account_id phai la so nguyen duong.\n");
                } else if (trungMa) {
                    printf("LOI: account_id %d da ton tai.\n", users[i].account_id);
                    hopLe = 0;
                }
            } while (!hopLe);

            do {
                printf("plan_type (0: Free, 1: Standard, 2: Premium): ");
                nhapOK = scanf("%d", &users[i].plan_type);
                while (getchar() != '\n');
                hopLe = (nhapOK == 1 && users[i].plan_type >= 0 && users[i].plan_type <= 2);
                if (!hopLe) {
                    printf("LOI: plan_type chi nhan 0, 1 hoac 2.\n");
                }
            } while (!hopLe);

            /* Cac truong duoi day chap nhan moi so nguyen - gia tri sai se duoc KIEM TOAN chuan hoa */
            do {
                printf("monthly_fee (VND, du lieu goc): ");
                nhapOK = scanf("%d", &users[i].monthly_fee);
                while (getchar() != '\n');
                if (nhapOK != 1) {
                    printf("LOI: Vui long nhap so nguyen.\n");
                }
            } while (nhapOK != 1);

            do {
                printf("remaining_days (co the am): ");
                nhapOK = scanf("%d", &users[i].remaining_days);
                while (getchar() != '\n');
                if (nhapOK != 1) {
                    printf("LOI: Vui long nhap so nguyen.\n");
                }
            } while (nhapOK != 1);

            do {
                printf("active_devices: ");
                nhapOK = scanf("%d", &users[i].active_devices);
                while (getchar() != '\n');
                if (nhapOK != 1) {
                    printf("LOI: Vui long nhap so nguyen.\n");
                }
            } while (nhapOK != 1);
        }
    }

    /* Luu vet du lieu goc + tinh MRR truoc kiem toan (theo phi dang ghi nhan) */
    for (i = 0; i < n; i++) {
        users[i].orig_plan    = users[i].plan_type;
        users[i].orig_fee     = users[i].monthly_fee;
        users[i].orig_days    = users[i].remaining_days;
        users[i].orig_devices = users[i].active_devices;
        users[i].flag_expired = users[i].flag_fee_fixed = 0;
        users[i].flag_dev_garbage = users[i].flag_dev_capped = 0;
        mrrTruoc += users[i].monthly_fee;
    }

    /* In bang du lieu goc */
    printf("\n===================== DU LIEU GOC (TRUOC KIEM TOAN) =====================\n");
    printf("%-4s | %-8s | %-10s | %12s | %10s | %9s\n",
           "STT", "ID", "GOI", "PHI (VND)", "NGAY CON", "THIET BI");
    printf("-------------------------------------------------------------------------\n");
    for (i = 0; i < n; i++) {
        printf("%-4d | %-8d | %-10s | %12d | %10d | %9d\n",
               i + 1, users[i].account_id,
               users[i].plan_type == 0 ? "Free" : (users[i].plan_type == 1 ? "Standard" : "Premium"),
               users[i].monthly_fee, users[i].remaining_days, users[i].active_devices);
    }

    /* ======================= BUOC 2: KIEM TOAN TUNG TAI KHOAN ======================= */
    for (i = 0; i < n; i++) {
        /* (1) Loi du lieu 3: so thiet bi rac */
        if (users[i].active_devices <= 0) {
            users[i].active_devices = 1;
            users[i].flag_dev_garbage = 1;
            soTbRac++;
        }

        /* (2) Quy tac het han (bao gom Loi du lieu 1: so ngay am) */
        if (users[i].remaining_days <= 0) {
            if (users[i].plan_type != 0) {
                users[i].flag_expired = 1;
                soHaCap++;
            }
            users[i].plan_type = 0;
            users[i].remaining_days = 0;        /* dua ve 0 de ghi nhan */
        } else {
            /* Quy tac vuot thiet bi: chi ap dung cho tai khoan CHUA het han */
            if (users[i].plan_type == 2) {
                gioiHan = TB_PREMIUM;
            } else if (users[i].plan_type == 1) {
                gioiHan = TB_STANDARD;
            } else {
                gioiHan = TB_FREE;
            }
            if (users[i].active_devices > gioiHan) {
                users[i].active_devices = gioiHan;
                users[i].flag_dev_capped = 1;
                soCatTb++;
            }
        }

        /* (3) Loi du lieu 2: chuan hoa phi theo plan_type SAU khi ha cap */
        if (users[i].plan_type == 2) {
            users[i].monthly_fee = PHI_PREMIUM;
        } else if (users[i].plan_type == 1) {
            users[i].monthly_fee = PHI_STANDARD;
        } else {
            users[i].monthly_fee = PHI_FREE;
        }
        /* Chi danh dau "chuan hoa phi" khi sai lech KHONG phai do ha cap gay ra */
        if (!users[i].flag_expired && users[i].monthly_fee != users[i].orig_fee) {
            users[i].flag_fee_fixed = 1;
            soChuanHoaPhi++;
        }

        mrrSau += users[i].monthly_fee;
    }

    /* ======================= BUOC 3: SAP XEP TAI CHO (GIAI PHAP A) =======================
     * Insertion Sort on dinh. temp dung truoc users[j] khi va chi khi:
     *   - temp tra phi va users[j] la Free, HOAC
     *   - ca hai tra phi va temp.fee > users[j].fee (so sanh NGHIEM NGAT -> giu thu tu goc khi bang nhau)
     * Hai tai khoan Free khong bao gio doi cho -> nhom Free giu nguyen thu tu xuat hien.
     * Dieu kien j >= 0 dat TRUOC khi truy cap users[j] -> khong vuot chi so du toan bo Free/tra phi.
     */
    for (i = 1; i < n; i++) {
        temp = users[i];
        j = i - 1;
        while (j >= 0) {
            truocNhau = 0;
            if (temp.plan_type > 0 && users[j].plan_type == 0) {
                truocNhau = 1;
            } else if (temp.plan_type > 0 && users[j].plan_type > 0
                       && temp.monthly_fee > users[j].monthly_fee) {
                truocNhau = 1;
            }
            if (!truocNhau) {
                break;
            }
            users[j + 1] = users[j];    /* gan struct: sao chep toan bo ban ghi */
            j--;
        }
        users[j + 1] = temp;
    }

    /* ======================= BUOC 4: XUAT KET QUA ======================= */
    for (i = 0; i < n; i++) {
        if (users[i].plan_type > 0) {
            soTraPhi++;
        }
    }

    printf("\n============================== KET QUA KIEM TOAN & SAP XEP ==============================\n");
    printf("%-4s | %-8s | %-10s | %12s | %10s | %9s | %s\n",
           "STT", "ID", "GOI", "PHI (VND)", "NGAY CON", "THIET BI", "GHI CHU KIEM TOAN");
    printf("-----------------------------------------------------------------------------------------\n");
    for (i = 0; i < n; i++) {
        if (i == soTraPhi && soTraPhi > 0 && soTraPhi < n) {
            printf("- - - - - - - - - - - - - - - - [ NHOM FREE / HA CAP ] - - - - - - - - - - - - - - - -\n");
        }
        printf("%-4d | %-8d | %-10s | %12d | %10d | %9d | ",
               i + 1, users[i].account_id,
               users[i].plan_type == 0 ? "Free" : (users[i].plan_type == 1 ? "Standard" : "Premium"),
               users[i].monthly_fee, users[i].remaining_days, users[i].active_devices);
        if (users[i].flag_expired) {
            printf("[HET HAN %d ngay -> Free] ", users[i].orig_days);
        }
        if (users[i].flag_fee_fixed) {
            printf("[Phi %d -> %d] ", users[i].orig_fee, users[i].monthly_fee);
        }
        if (users[i].flag_dev_garbage) {
            printf("[TB rac %d -> 1] ", users[i].orig_devices);
        }
        if (users[i].flag_dev_capped) {
            printf("[Cat TB %d -> %d] ",
                   users[i].flag_dev_garbage ? 1 : users[i].orig_devices, users[i].active_devices);
        }
        if (!users[i].flag_expired && !users[i].flag_fee_fixed
            && !users[i].flag_dev_garbage && !users[i].flag_dev_capped) {
            printf("Hop le");
        }
        printf("\n");
    }
    printf("-----------------------------------------------------------------------------------------\n");

    printf("\n========================= BAO CAO MRR =========================\n");
    printf("So tai khoan            : %d (tra phi: %d | Free/ha cap: %d)\n", n, soTraPhi, n - soTraPhi);
    printf("Bi ha cap do het han    : %d\n", soHaCap);
    printf("Chuan hoa phi sai lech  : %d\n", soChuanHoaPhi);
    printf("Sua so thiet bi rac     : %d\n", soTbRac);
    printf("Cat giam thiet bi vuot  : %d\n", soCatTb);
    printf("---------------------------------------------------------------\n");
    printf("MRR truoc kiem toan      : %15lld VND\n", mrrTruoc);
    printf("MRR thuc te sau kiem toan: %15lld VND\n", mrrSau);
    printf("Chenh lech (sau - truoc) : %+15lld VND", mrrSau - mrrTruoc);
    if (mrrTruoc != 0) {
        printf("  (%+.2f%%)", 100.0 * (double)(mrrSau - mrrTruoc) / (double)mrrTruoc);
    }
    printf("\n===============================================================\n");

    return 0;
}
