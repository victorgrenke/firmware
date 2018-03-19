/**
  Copyright (c) 2015 Particle Industries, Inc.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */


#ifndef SYSTEM_CLOUD_INTERNAL_H
#define	SYSTEM_CLOUD_INTERNAL_H

#include "system_cloud.h"

#include "spark_wiring_diagnostics.h"

/**
 * Functions for managing the cloud connection, performing cloud operations
 * and system upgrades.
 */

int Internet_Test(void);

void spark_cloud_udp_port_set(uint16_t port);
int spark_cloud_socket_connect(void);
int spark_cloud_socket_disconnect(bool graceful=true);

void Spark_Protocol_Init(void);
int Spark_Handshake(bool presence_announce);
bool Spark_Communication_Loop(void);
void Multicast_Presence_Announcement(void);
void Spark_Signal(bool on, unsigned, void*);
void Spark_SetTime(unsigned long dateTime);
void Spark_Process_Events();
void Spark_Sleep();
void Spark_Wake();
void Spark_Abort();

void system_set_time(time_t time, unsigned param, void* reserved);

String bytes2hex(const uint8_t* buf, unsigned len);

uint8_t spark_cloud_socket_closed();

bool spark_function_internal(const cloud_function_descriptor* desc, void* reserved);
int call_raw_user_function(void* data, const char* param, void* reserved);

String spark_deviceID();

struct User_Var_Lookup_Table_t
{
    const void *userVar;
    Spark_Data_TypeDef userVarType;
    char userVarKey[USER_VAR_KEY_LENGTH+1];

    const void* (*update)(const char* name, Spark_Data_TypeDef varType, const void* var, void* reserved);
};


struct User_Func_Lookup_Table_t
{
    void* pUserFuncData;
    cloud_function_t pUserFunc;
    char userFuncKey[USER_FUNC_KEY_LENGTH+1];
};


User_Var_Lookup_Table_t* find_var_by_key_or_add(const char* varKey);
User_Func_Lookup_Table_t* find_func_by_key_or_add(const char* funcKey);

extern ProtocolFacade* sp;


/**
 * regular async update to check that the cloud has been serviced recently.
 * After 15 seconds of inactivity, the LED status is changed to
 * @return
 */
bool system_cloud_active();

namespace particle {

class CloudDiagnostics {
public:
    // Note: Use odd numbers to encode transitional states
    enum Status {
        DISCONNECTED = 0,
        CONNECTING = 1,
        CONNECTED = 2,
        DISCONNECTING = 3
    };

    CloudDiagnostics() :
            status_(DIAG_ID_CLOUD_CONNECTION_STATUS, DIAG_NAME_CLOUD_CONNECTION_STATUS, DISCONNECTED),
            disconnReason_(DIAG_ID_CLOUD_DISCONNECTION_REASON, DIAG_NAME_CLOUD_DISCONNECTION_REASON, CLOUD_DISCONNECT_REASON_NONE),
            disconnCount_(DIAG_ID_CLOUD_DISCONNECTS, DIAG_NAME_CLOUD_DISCONNECTS),
            connCount_(DIAG_ID_CLOUD_CONNECTION_ATTEMPTS, DIAG_NAME_CLOUD_CONNECTION_ATTEMPTS),
            lastError_(DIAG_ID_CLOUD_CONNECTION_ERROR_CODE, DIAG_NAME_CLOUD_CONNECTION_ERROR_CODE) {
    }

    CloudDiagnostics& status(Status status) {
        status_ = status;
        return *this;
    }

    CloudDiagnostics& connectionAttempt() {
        ++connCount_;
        return *this;
    }

    CloudDiagnostics& resetConnectionAttempts() {
        connCount_ = 0;
        return *this;
    }

    CloudDiagnostics& disconnectionReason(cloud_disconnect_reason reason) {
        disconnReason_ = reason;
        return *this;
    }

    CloudDiagnostics& disconnectedUnexpectedly() {
        ++disconnCount_;
        return *this;
    }

    CloudDiagnostics& lastError(int error) {
        lastError_ = error;
        return *this;
    }

    static CloudDiagnostics* instance();

private:
    // Some of the diagnostic data sources use the synchronization since they can be updated from
    // the networking service thread
    AtomicEnumDiagnosticData<Status> status_;
    AtomicEnumDiagnosticData<cloud_disconnect_reason> disconnReason_;
    SimpleIntegerDiagnosticData disconnCount_;
    SimpleIntegerDiagnosticData connCount_;
    SimpleIntegerDiagnosticData lastError_;
};

} // namespace particle


#endif	/* SYSTEM_CLOUD_INTERNAL_H */

