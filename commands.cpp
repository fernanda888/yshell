// $Id: commands.cpp,v 1.19 2020-10-20 18:23:13-07 - - $

#include <cctype>
#include <iomanip>
#include "commands.h"
#include "debug.h"

command_hash cmd_hash {
   {"cat"   , fn_cat   },//done
   {"cd"    , fn_cd    },//done
   {"echo"  , fn_echo  },//done
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },//done
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },//done
   {"mkdir" , fn_mkdir },//done
   {"prompt", fn_prompt},//done
   {"pwd"   , fn_pwd   },//done
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
   {"#"     , fn_hash   },//done
};

pair<inode_ptr,string> trvrs_path(inode_state& state, const wordvec& words){
  inode_ptr find_dir=state.get_cwd();
  wordvec path=split(words.at(1),"/");
    for(auto it=path.begin();it!=(path.end()-1);it++){
      if(find_dir->is_dir()==true && find_dir->get_contents()->dir_exists(*it)==true){
       inode_ptr new_dir=find_dir->get_contents()->get_pointer(*it);
       find_dir=new_dir; 
      }
      else{
        throw command_error (words.at(0)+ ": "+ words.at(1) 
        + ": No such file or directory");
      }
    }
  pair found_path(find_dir,path.at((path.size()-1)));
  return found_path;
}

bool path_exists(string words){
  if(words.find_first_of('/')!=string::npos){
    return true;
  }
  else{
    return false;
  }
}

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int status = exec::status();
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size()==1){
     throw command_error (words.at(0) + ": No file specified");
   }
   bool exists= state.get_cwd()->get_contents()->dir_exists(words.at(1));
   if(exists==false){
    throw command_error (words.at(0)+ ": "+ words.at(1) 
        + ": No such file or directory");
   }
   inode_ptr new_cd=state.get_cwd()->get_contents()->get_pointer(words.at(1));
   if(new_cd->is_dir()==true){
      throw command_error (words.at(0)+ ": "+
        words.at(1) + ": Not a file");
   }
   inode_ptr find_file=state.get_cwd()->get_contents()->get_pointer(words.at(1));
   wordvec file=find_file->get_contents()->readfile();
   for(auto it=file.begin();it!=file.end();++it){
     cout<<*it + " ";
   }
   cout<<"\n";
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size()==1){
     state.set_cwd(state.get_root());
   }
   else{
     bool exists= state.get_cwd()->get_contents()->dir_exists(words.at(1));
     if(exists==false){
      throw command_error (words.at(0)+ ": "+ words.at(1) 
      + ": No such file or directory");
     }
     else{
       inode_ptr new_cd=state.get_cwd()->get_contents()->get_pointer(words.at(1));
       if(new_cd->is_dir()==false){
         throw command_error (words.at(0)+ ": "+
         words.at(1) + ": Not a directory");
       }
       state.set_cwd(new_cd);
       state.set_pwd(words.at(1));
     }
}
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   int stat;
   if(words.size()==1){
     exec::status(0);
   }
   if(words.size()>1){
     stat=stoi(words.at(1), nullptr, 10);
     exec::status (stat);
   }
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr curr_ptr=nullptr;
   if(words.size()==1){
     curr_ptr=state.get_cwd();
   }
   else if(words.at(1)=="/"){
     curr_ptr= state.get_root();
   }
   else{
     bool exists= state.get_cwd()->get_contents()->dir_exists(words.at(1));
     if(exists==false){
      throw command_error (words.at(0)+ ": "+ words.at(1) 
      + ": No such file or directory");
     }
     curr_ptr=state.get_cwd()->get_contents()->get_pointer(words.at(1));
   }
   if(state.get_cwd()==state.get_root()){
     cout<<"/:"<<endl;
   }
   else{
     cout<<"/"+state.get_pwd()+":"<<endl;
   }
   wordvec list;
   list=curr_ptr->get_contents()->get_map_names();
   cout<<setw(6);
   for(auto it=list.begin();it!=list.end();++it){
     inode_ptr curr_dir=curr_ptr->get_contents()->get_pointer(*it);
     cout<<setw(6);
     cout<<curr_dir->get_inode_nr();
     cout<<setw(6);
     cout<<curr_dir->get_contents()->size();
     cout<<"  ";
     cout<<*it;
     if(curr_dir->is_dir()==true){
       cout<<"/";
     }
     cout<<"\n";
   }
}
void print_map(inode_ptr curr_ptr){
  wordvec list;
   list=curr_ptr->get_contents()->get_map_names();
   cout<<setw(6);
   for(auto it=list.begin();it!=list.end();++it){
     inode_ptr curr_dir=curr_ptr->get_contents()->get_pointer(*it);
     cout<<setw(6);
     cout<<curr_dir->get_inode_nr();
     cout<<setw(6);
     cout<<curr_dir->get_contents()->size();
     cout<<"  ";
     cout<<*it;
     if(curr_dir->is_dir()==true){
       cout<<"/";
     }
     cout<<"\n";
   }
}

void lsr_helper(inode_ptr curr_ptr, string dir_name){
   cout<<dir_name+":"<<endl;
   print_map(curr_ptr);
   wordvec list;
   list=curr_ptr->get_contents()->get_map_names();
  for(auto it=list.begin()+2;it!=list.end();++it){
    inode_ptr curr_dir=curr_ptr->get_contents()->get_pointer(*it);
    if(curr_dir->is_dir()==true){
       lsr_helper(curr_dir,dir_name+"/"+*it);
     }
  }
}
void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr curr_ptr=nullptr;
   if(words.size()==1){
     curr_ptr=state.get_cwd();
   }
   else if(words.at(1)=="/"){
     curr_ptr= state.get_root();
   }
   else{
     bool exists= state.get_cwd()->get_contents()->dir_exists(words.at(1));
     if(exists==false){
      throw command_error (words.at(0)+ ": "+ words.at(1) 
      + ": No such file or directory");
     }
     curr_ptr=state.get_cwd()->get_contents()->get_pointer(words.at(1));
   }
   if(words.at(1)=="/")
    lsr_helper(curr_ptr, "");
   else
     lsr_helper(curr_ptr, words.at(1));
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr working_dir=nullptr;
   inode_ptr new_file=nullptr;
   string file_name="";
   if(words.size()==1){
     throw command_error (words.at(0) + ": No file specified");
   }
   if(path_exists(words.at(1))==true){
     pair dir_and_file=trvrs_path(state,words);
     working_dir=dir_and_file.first;
     file_name=dir_and_file.second;
   }
   else{
     working_dir=state.get_cwd();
     file_name=words.at(1);
   }
   if(working_dir->get_contents()->dir_exists(file_name)==true){
     if(working_dir->get_contents()->get_pointer(file_name)->is_dir()){
       throw command_error (words.at(0)+ ": "+
         words.at(1) + ": Not a file");
     }else{
       new_file=working_dir->get_contents()->get_pointer(file_name);
     }
   }
   else{
     new_file=make_shared<inode>(inode(file_type::PLAIN_TYPE));
   }
   wordvec new_data;
   working_dir->get_contents()->insert_dir(file_name, new_file);
   for(auto it=words.begin()+2;it!=words.end();it++){
     new_data.push_back(*it);
   }
   new_file->get_contents()->writefile(new_data);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string dir_name="";
   inode_ptr working_dir=nullptr;
   if(words.size()==1){
     throw command_error (words.at(0) + ": No directory specified");
   }
   if(path_exists(words.at(1))==true){
     pair dir_and_file=trvrs_path(state,words);
     working_dir=dir_and_file.first;
     dir_name=dir_and_file.second;
   }
   else{
     working_dir=state.get_cwd();
     dir_name=words.at(1);
   }
   if(working_dir->get_contents()->dir_exists(dir_name)==true){
     throw command_error (words.at(0)+ ": "+
         words.at(1) + ": Already exists");
   }
   working_dir->get_contents()->mkdir(dir_name, working_dir);
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string new_prompt="";
   for(auto it=words.begin()+1;it!=words.end();++it){
     new_prompt+=*it;
     new_prompt+=" ";
   }
   state.prompt(new_prompt);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(state.get_cwd()==state.get_root()){
     cout<<"/"<<endl;
   }
   else{
     cout<<"/"+state.get_pwd()<<endl;
   }
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size()==1){
     throw command_error (words.at(0) + ": No file specified");
   }
   bool exists= state.get_cwd()->get_contents()->dir_exists(words.at(1));
   if(exists==false){
     throw command_error (words.at(0)+ ": "+ words.at(1) 
     + ": No such file or directory");
   }
   inode_ptr rm_dir=state.get_cwd()->get_contents()->get_pointer(words.at(1));
   if(rm_dir->is_dir()==true){
     if(rm_dir->get_contents()->size()==2){
       rm_dir->get_contents()->clear_map();
     }
     else{
        throw command_error (words.at(0)+ ": "+ words.at(1) 
        + "directory not empty");
     }
   }
   state.get_cwd()->get_contents()->remove(words.at(1));
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size()==1){
     throw command_error (words.at(0) + ": No file specified");
   }
   bool exists= state.get_cwd()->get_contents()->dir_exists(words.at(1));
   if(exists==false){
     throw command_error (words.at(0)+ ": "+ words.at(1)
     + ": No such file or directory");
   }
   inode_ptr rm_dir=state.get_cwd()->get_contents()->get_pointer(words.at(1));
   if(rm_dir->is_dir()==true){
     rm_dir->get_contents()->clear_map();
   }
   state.get_cwd()->get_contents()->remove(words.at(1));
}
void fn_hash (inode_state& state, const wordvec& words){
  state.set_cwd(state.get_cwd());
  if(words.at(0)=="no warning"){
  }
}

