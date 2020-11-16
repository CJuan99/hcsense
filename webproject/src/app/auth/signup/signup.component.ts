import { Component, OnInit } from '@angular/core';
import {NgForm} from '@angular/forms';
import {AuthService} from '../auth.services';
@Component({
  selector: 'app-signup',
  templateUrl: './signup.component.html',
  styleUrls: ['./signup.component.css']
})
export class SignupComponent implements OnInit {
  constructor(public authService: AuthService) { }

  onSignUp(form: NgForm){
    //console.log(form.value);
    if (form.invalid){
      return;
    }
    this.authService.createdUser(form.value.email,form.value.password);
  }




  ngOnInit() {
  }

}
