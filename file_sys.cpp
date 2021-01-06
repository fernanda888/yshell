// $Id: file_sys.cpp,v 1.9 2020-10-26 21:32:08-07 - - $

#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace std;

#include "debug.h"
#include "file_sys.h"

size_t inode::next_inode_nr {1};

ostream& operator<< (ostream& out, file_type type) {
   switch (type) {
      case file_type::PLAIN_TYPE: cout << "PLAIN_TYPE"; break;
      case file_type::DIRECTORY_TYPE: cout << "DIRECTORY_TYPE"; break;
      default: assert (false);
   };
   return out;
}

inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
  root=make_shared<inode>(inode(file_type::DIRECTORY_TYPE));
  root->parent=root;
  root->is_root=true;
  root->contents->make_map(root);
  cwd=root;
}

void inode_state::set_cwd(inode_ptr cd){
  cwd=cd;
}
void inode_state:: set_pwd(string curr_dir){
  pwd_=curr_dir;
}
string inode_state:: get_pwd(){
  return pwd_;
}
inode_ptr inode_state:: get_root(){
  return root;
}
inode_ptr inode_state:: get_cwd(){
  return cwd;
}
void inode_state::prompt (string new_prompt){
  prompt_=new_prompt+" ";
}
const string& inode_state::prompt() const { return prompt_; }

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           is_dir_=false;
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           is_dir_=true;
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

size_t inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}
bool inode:: is_dir(){
  return is_dir_;
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

base_file_ptr inode:: get_contents(){
  return contents;
}
const wordvec& base_file::readfile() const {
   throw file_error ("is a " + error_file_type());
}

void base_file::writefile (wordvec) {
   throw file_error ("is a " + error_file_type());
}
bool base_file:: dir_exists(string){
  throw file_error ("is a " + error_file_type());
}
void base_file:: clear_map(){
  throw file_error ("is a " + error_file_type());
}
void base_file::remove (const string&) {
   throw file_error ("is a " + error_file_type());
}

void base_file::make_map (const inode_ptr) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkdir (const string&, inode_ptr) {
   throw file_error ("is a " + error_file_type());
}

wordvec base_file:: get_map_names(){
   throw file_error ("is a " + error_file_type());
}
inode_ptr base_file:: get_pointer(string){
   throw file_error ("is a " + error_file_type());
}
void base_file:: insert_dir(string, inode_ptr){
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkfile (const string&) {
   throw file_error ("is a " + error_file_type());
}



size_t plain_file::size() const {
   size_t size {0};
   for(int iter=0;iter<data.size();++iter){
     size+=data.at(iter).size();
   }
   DEBUGF ('i', "size = " << size);
   size+=(data.size()-1);
   return size;
}
const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}
void plain_file::writefile (wordvec words) {
   DEBUGF ('i', words);
   data=words;
}

size_t directory::size() const {
    size_t size= dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

void directory:: make_map(const inode_ptr dir_inode){
  //only called after parent is set
  dir_inode->get_contents()->insert_dir(".",dir_inode);
  dir_inode->get_contents()->insert_dir("..",dir_inode->parent);
}

void directory:: insert_dir(string dir_name, inode_ptr dir_ptr){
  this->dirents.insert({dir_name, dir_ptr});
}

wordvec directory:: get_map_names(){
  wordvec map_names;
  for(auto it=dirents.begin(); it!=dirents.end(); ++it){
    map_names.push_back(it->first);
  }
  return map_names;
}
inode_ptr directory:: get_pointer(string dirent){
  return this->dirents.at(dirent);
}
bool directory:: dir_exists(string dir_name){
  const auto result = this->dirents.find (dir_name);
  if (result == dirents.end()) {
      return false;
   }else{
     return true;
   }
}
void directory:: clear_map(){
  this->dirents.erase(dirents.begin(),dirents.end());
}
void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   this->dirents.erase(filename);
}

inode_ptr directory::mkdir (const string& dirname, inode_ptr curr_dir){
  //dir name needs to be added to current directories map bc a dir doesn't need to know its own name
  //parent is always cwd
   DEBUGF ('i', dirname);
   inode_ptr new_dir=make_shared<inode>(inode(file_type::DIRECTORY_TYPE));
   this->dirents.insert({dirname , new_dir});
   new_dir->parent=curr_dir;
   make_map(new_dir);
   return new_dir;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   return nullptr;
}

