/*
Copyright 2021.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package deviceInfo

import (
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
)

// EDIT THIS FILE!  THIS IS SCAFFOLDING FOR YOU TO OWN!
// NOTE: json tags are required.  Any new fields you add must have json tags for the fields to be serialized.

// MLUSpec defines the desired state of MLU
type MLUSpec struct {
	// INSERT ADDITIONAL SPEC FIELDS - desired state of cluster
	// Important: Run "make" to regenerate code after modifying this file

	UUID string `json:"uuid,omitempty"`

	//Model string `json:"model,omitempty"`

	//Family string `json:"family,omitempty"`

	Capacity R `json:"capacity,omitempty"`

	Used R `json:"used,omitempty"`

	Node string `json:"node,omitempty"`
}

// MLUStatus defines the observed state of MLU
type MLUStatus struct {
	// INSERT ADDITIONAL STATUS FIELD - define observed state of cluster
	// Important: Run "make" to regenerate code after modifying this file
}

// +genclient
// +genclient:nonNamespaced
// +kubebuilder:object:root=true
// +kubebuilder:printcolumn:name="node",type="string",JSONPath=".spec.node",description="NODE"
// +kubebuilder:printcolumn:name="model",type="string",JSONPath=".spec.model",description="MODEL"
// +kubebuilder:printcolumn:name="core used",type="string",JSONPath=".spec.used.core",description="CORE"
// +kubebuilder:printcolumn:name="memory used",type="string",JSONPath=".spec.used.memory",description="MEMORY USED"
// +kubebuilder:printcolumn:name="memory capacity",type="string",JSONPath=".spec.capacity.memory",description="MEMORY CAPACITY"
// +kubebuilder:subresource:status
// MLU is the Schema for the MLUs API
type MLU struct {
	metav1.TypeMeta   `json:",inline"`
	metav1.ObjectMeta `json:"metadata,omitempty"`

	Spec   MLUSpec   `json:"spec,omitempty"`
	Status MLUStatus `json:"status,omitempty"`
}

// +kubebuilder:object:root=true

// MLUList contains a list of MLU
type MLUList struct {
	metav1.TypeMeta `json:",inline"`
	metav1.ListMeta `json:"metadata,omitempty"`
	Items           []MLU `json:"items"`
}
