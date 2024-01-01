#ifndef DATABAZOVY_SYSTEM_PERMISSION_H
#define DATABAZOVY_SYSTEM_PERMISSION_H


#include <string>


enum PermissionType {
    select_permission,
    insert_permission,
    update_permission,
    delete_permission
};

class Permission {
public:
    static std::string permissionTypeToString(PermissionType permission) {
        switch (permission) {
            case select_permission:
                return "select";
            case insert_permission:
                return "insert";
            case update_permission:
                return "update";
            case delete_permission:
                return "delete";
            default:
                return "";
        }
    }
};

#endif //DATABAZOVY_SYSTEM_PERMISSION_H
