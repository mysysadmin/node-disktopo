#include <v8.h>
#include <node.h>
#include <libnvpair.h>
#include <fm/libtopo.h>
#include <strings.h>
#include <errno.h>
#include <iostream>
#include <list>

typedef struct disk_topo {
	char *hdc ;
	char *product_id ;
	char *chassis_id ;
	char *logical_disk ;
	char *manufacturer ;
	char *model ;
	char *serial_number ;
	char *firmware_rev ;
	char *capacity ;
	char *slot ;
} disk_topo_t ;

using namespace v8;

class Disktopo : node::ObjectWrap {

	public:
		static void Initialize(Handle<Object> target);

	protected:
		Disktopo() ;
		~Disktopo() ;

		static Handle<Value> New(const Arguments& args) ;
		static Handle<Value> Version(const Arguments& args) ;
		static Handle<Value> ListTopo(const Arguments& args) ;

	private:
		topo_hdl_t *topo_handle ;
		char *uuid ;
		static Persistent<FunctionTemplate> pftpl ;

} ;

Persistent<FunctionTemplate> Disktopo::pftpl ;

Disktopo::Disktopo() : node::ObjectWrap()
{
	int err = 0 ;
	if((topo_handle = topo_open(TOPO_VERSION , "/" , &err)) == NULL) {
		throw(topo_strerror(err)) ;
	}

	if ((uuid = topo_snap_hold(topo_handle , NULL , &err)) == NULL) {
		topo_close(topo_handle) ;
		throw(topo_strerror(err)) ;
	}
}

Disktopo::~Disktopo()
{
	if(topo_handle != NULL && uuid != NULL) {
		topo_snap_release(topo_handle) ;	
		topo_hdl_strfree(topo_handle , uuid) ;
		topo_close(topo_handle) ;
	}
}

void Disktopo::Initialize(Handle<Object> target)
{
	HandleScope scope;
	Local<FunctionTemplate> ftpl = FunctionTemplate::New(Disktopo::New);
	pftpl = Persistent<FunctionTemplate>::New(ftpl);
	pftpl->InstanceTemplate()->SetInternalFieldCount(1);
	pftpl->SetClassName(String::NewSymbol("topohandle"));
	NODE_SET_PROTOTYPE_METHOD(pftpl , "version" , Disktopo::Version);
	NODE_SET_PROTOTYPE_METHOD(pftpl , "list" , Disktopo::ListTopo);
	target->Set(String::NewSymbol("topohandle"), pftpl->GetFunction());
}

Handle<Value> Disktopo::New(const Arguments& args)
{
	HandleScope scope;
	Disktopo *topo ;

	try {
		topo = new Disktopo();
	} catch (char const *msg) {
		return (ThrowException(Exception::Error(String::New(msg))));
	}

	topo->Wrap(args.Holder());

	return (args.This());
}

Handle<Value> Disktopo::Version(const Arguments &args)
{
	double version = 0.1 ;
	return(Number::New(version)) ;
}

int walk_cb(topo_hdl_t *handle , tnode_t *node , void *data)
{
	std::list<disk_topo_t> *plist = (std::list<disk_topo_t>*)data ;
	int err = 0 ;
	nvlist_t *list = NULL ;
	disk_topo_t disktopo ;
	if(topo_node_resource(node , &list , &err) < 0) {
		//fprintf(stderr , "topo_node_resource failed %s\n" , topo_strerror(err)) ;
		return(TOPO_WALK_NEXT) ;
	} else {
		if(topo_fmri_nvl2str(handle , list , &disktopo.hdc, &err) < 0) {
			disktopo.hdc = NULL ;
		} else {
			if(strstr(disktopo.hdc , "disk=") != NULL) {
				if(topo_prop_get_string(node , "authority" , "product-id" , &disktopo.product_id , &err) < 0) {
					disktopo.product_id = NULL ;
				} 		

				if(topo_prop_get_string(node , "authority" , "chassis-id" , &disktopo.chassis_id , &err) < 0) {
					disktopo.chassis_id = NULL ;
				}

				if(topo_prop_get_string(node , "storage" , "logical-disk" , &disktopo.logical_disk , &err) < 0) {
					disktopo.logical_disk = NULL ;
				}

				if(topo_prop_get_string(node , "storage" , "manufacturer" , &disktopo.manufacturer , &err) < 0) {
					disktopo.manufacturer = NULL ;
				}
				if(topo_prop_get_string(node , "storage" , "model" , &disktopo.model , &err) < 0) {
					disktopo.model = NULL ;
				}
				if(topo_prop_get_string(node , "storage" , "serial-number" , &disktopo.serial_number , &err) < 0) {
					disktopo.serial_number = NULL ;
				}
				if(topo_prop_get_string(node , "storage" , "firmware-revision" , &disktopo.firmware_rev , &err) < 0) {
					disktopo.firmware_rev = NULL ;
				}
				if(topo_prop_get_string(node , "storage" , "capacity-in-bytes" , &disktopo.capacity , &err) < 0) {
					disktopo.capacity = NULL ;
				}
				if(topo_prop_get_string(node , "protocol" , "label" , &disktopo.slot , &err) < 0) {
					disktopo.slot = NULL ;
				}

				plist->push_back(disktopo) ;

				if(disktopo.hdc != NULL)
					topo_hdl_strfree(handle , disktopo.hdc) ;

				if(disktopo.product_id != NULL)
					topo_hdl_strfree(handle , disktopo.product_id) ;

				if(disktopo.chassis_id != NULL)
					topo_hdl_strfree(handle , disktopo.chassis_id) ;

				if(disktopo.logical_disk != NULL)
					topo_hdl_strfree(handle , disktopo.logical_disk) ;

				if(disktopo.manufacturer != NULL)
					topo_hdl_strfree(handle , disktopo.manufacturer) ;

				if(disktopo.model != NULL)
					topo_hdl_strfree(handle , disktopo.model) ;

				if(disktopo.serial_number != NULL)
					topo_hdl_strfree(handle , disktopo.serial_number) ;

				if(disktopo.firmware_rev != NULL)
					topo_hdl_strfree(handle , disktopo.firmware_rev) ;

				if(disktopo.capacity != NULL)
					topo_hdl_strfree(handle , disktopo.capacity) ;

				if(disktopo.slot != NULL)
					topo_hdl_strfree(handle , disktopo.slot) ;

				nvlist_free(list) ;
				return(TOPO_WALK_NEXT) ;
			}

			nvlist_free(list) ;
			return(TOPO_WALK_NEXT) ;
		}

		nvlist_free(list) ;
		return(TOPO_WALK_NEXT) ;
	}

	return(TOPO_WALK_NEXT) ;
}

Handle<Value> Disktopo::ListTopo(const Arguments &args)
{
	HandleScope scope;
	int j = 0 ;
	int err = 0 ;
	topo_walk_t *walker = NULL ;
	std::list<disk_topo_t> *plist = new std::list<disk_topo_t>() ;
	Local<Object> disks = Object::New() ;
	Local<Array> diskNames = Array::New() ;
	Disktopo *obj = ObjectWrap::Unwrap<Disktopo>(args.This());

	if((walker = topo_walk_init(obj->topo_handle , "hc" , walk_cb , plist , &err)) != NULL) {
		if(topo_walk_step(walker , TOPO_WALK_CHILD) == TOPO_WALK_ERR) 
			return(Object::New()) ;
		topo_walk_fini(walker) ;
	} else {
		return(Object::New()) ;
	}

	std::list<disk_topo_t>::iterator i ;
	for(i = plist->begin() ; i != plist->end() ; ++i , j++) {
		Local<Object> o = Object::New() ;
		o->Set(String::NewSymbol("Product-Id"), String::New(i->product_id));
		o->Set(String::NewSymbol("Chassis-Id"), String::New(i->chassis_id));
		o->Set(String::NewSymbol("Logical-disk"), String::New(i->logical_disk));
		o->Set(String::NewSymbol("Manufacturer"), String::New(i->manufacturer));
		o->Set(String::NewSymbol("Model"), String::New(i->model));
		o->Set(String::NewSymbol("Serial-Number"), String::New(i->serial_number));
		o->Set(String::NewSymbol("Firmware-Rev"), String::New(i->firmware_rev));
		o->Set(String::NewSymbol("Capacity"), String::New(i->capacity));
		o->Set(String::NewSymbol("Slot"), String::New(i->slot));
		diskNames->Set(j , o) ;
	}

	disks->Set(String::NewSymbol("topology"), diskNames);
	delete plist ;
	return scope.Close(disks) ;
}

extern "C" void
init (Handle<Object> target)
{
	Disktopo::Initialize(target);
}

