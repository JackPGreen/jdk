/*
 * Copyright (c) 2021, 2024, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#include <string.h>
#include "jvmti.h"
#include "jvmti_common.hpp"

extern "C" {

static jvmtiEnv *jvmti = nullptr;
// JVMTI_ERROR_WRONG_PHASE guard
static jrawMonitorID event_mon = nullptr;
static bool is_vm_dead = false;

static int number_of_allocation = 0;

extern JNIEXPORT void JNICALL
VMObjectAlloc(jvmtiEnv *jvmti,
              JNIEnv* jni,
              jthread thread,
              jobject object,
              jclass cls,
              jlong size) {
  RawMonitorLocker locker(jvmti, jni, event_mon);
  if (is_vm_dead) {
    return;
  }

  char *signature = nullptr;
  jvmtiError err = jvmti->GetClassSignature(cls, &signature, nullptr);
  if (err != JVMTI_ERROR_NONE) {
    jni->FatalError("Failed during the GetClassSignature call");
  }

  LOG("VMObjectAlloc called for %s\n", signature);

  if (!strcmp(signature, "LVMObjectAllocTest;")) {
    number_of_allocation++;
  }
}

static void JNICALL
VMDeath(jvmtiEnv *jvmti, JNIEnv* jni) {
  RawMonitorLocker locker(jvmti, jni, event_mon);

  LOG("VMDeath\n");
  is_vm_dead = true;
}

JNIEXPORT jint JNICALL
Java_VMObjectAllocTest_getNumberOfAllocation(JNIEnv *env, jclass cls) {
  return number_of_allocation;
}

extern JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *jvm, char *options, void *reserved) {
  jvmtiEventCallbacks callbacks;
  jvmtiError err;
  jvmtiCapabilities caps;

  if (jvm->GetEnv((void **) &jvmti, JVMTI_VERSION) != JNI_OK) {
    return JNI_ERR;
  }

  err = jvmti->CreateRawMonitor("Event Monitor", &event_mon);
  if (err != JVMTI_ERROR_NONE) {
    LOG("Agent_OnLoad: CreateRawMonitor failed: %d\n", err);
    return JNI_ERR;
  }

  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.VMObjectAlloc = &VMObjectAlloc;
  callbacks.VMDeath = &VMDeath;
  memset(&caps, 0, sizeof(caps));
  caps.can_generate_vm_object_alloc_events = 1;

  err = jvmti->AddCapabilities( &caps);
  if (err != JVMTI_ERROR_NONE) {
    return JNI_ERR;
  }

  err = jvmti->SetEventCallbacks(&callbacks, sizeof(jvmtiEventCallbacks));
  if (err != JVMTI_ERROR_NONE) {
    return JNI_ERR;
  }

  err = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_OBJECT_ALLOC , nullptr);
  if (err != JVMTI_ERROR_NONE) {
    return JNI_ERR;
  }
  err = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_DEATH, nullptr);
  if (err != JVMTI_ERROR_NONE) {
    return JNI_ERR;
  }

  return JNI_OK;
}

} //extern "C"
