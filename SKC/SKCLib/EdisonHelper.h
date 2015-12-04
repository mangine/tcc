#ifndef EDISONHELPER_H_INCLUDED
#define EDISONHELPER_H_INCLUDED

class EdisonHelper{
public:
    static void DisableWiFi(){
        cout << "[EdisonHelper] Shutting down WiFi" << endl;
        system("systemctl stop wpa_supplicant");
    }

    static void EnableWiFi(){
        cout << "[EdisonHelper] Powering up WiFi" << endl;
        system("systemctl restart wpa_supplicant");
    }

    static void EnableBluetooth(){
        cout << "[EdisonHelper] Powering up bluetooth" << endl;
        system("rfkill unblock bluetooth");
    }

    static void DisableBluetooth(){
        cout << "[EdisonHelper] Shutting down Bluetooth" << endl;
        system("rfkill block bluetooth");
    }

};

#endif // EDISONHELPER_H_INCLUDED
