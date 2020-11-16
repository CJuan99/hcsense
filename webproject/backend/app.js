const express = require('express')
const bodyParser = require('body-parser')

const mongoose = require("mongoose"); //import mongoose
const app = express()
const bcrypt =require ("bcrypt");
const User = require ("./models/user");
const jwt = require('jsonwebtoken');
const checkAuth =require("./middleware/check-auth");

mongoose.connect("mongodb://localhost:27017/hcsense")


.then(()=>{
  console.log('Connected to database');
})
.catch(()=>{
  console.log('Connection failed');
});


app.use(bodyParser.json());

app.use((req, res, next)=>{
res.setHeader("Access-Control-Allow-Origin", "*");
res.setHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Authorization");
res.setHeader("Access-Control-Allow-Methods", "GET, POST, PATCH, PUT, DELETE, OPTIONS");
next();
});


app.post('/api/user/signup', (req, res, next)=>{
  bcrypt.hash(req.body.password,10)
  .then(hash =>{
    const user = new User({
      email: req.body.email,
      password: hash
    });
    user.save() //save data to database
    .then(result =>{
      res.status(201).json({
        message: 'User created', //response after saving
        result:result
      });
    })
    .catch(err =>{
      res.status(500).json({
        error:err
      });
    });
  });
});


// app.post("/api/posts", checkAuth,(req,res, next)=>
// {
//   //const post = req.body;

//   const post = new Post({
//     title: req.body.title,
//     content: req.body.content
//   });
//   post.save().then(createdPost => {
//     console.log(post)
//     res.status(200).json({
//       message:'Post added successfully',
//       postId: createdPost._id
//     });
//   });

//  /* console.log(post);
//   res.status(201).json({
//     message:'Post added successfully'
//   });*/
// });

// /*
// app.use((req, res, next)=>{
//   console.log ('First middleware')
//   next()
// }
// )

// app.use((req, res, next)=> {
//   console.log ('Second middlewaree')
//   next()
// })

// app.use((req, res, next)=> {
//   res.send ('Hello from express!')
// })

// module.exports = app;
// */
// app.put("/api/posts/:id", checkAuth, (req,res, next)=>{
//   const post = new Post({
//     _id: req.body.id,
//     title:req.body.title,
//     content:req.body.content
//   });
//   Post.updateOne({_id: req.params.id},post).then(result => {
//     console.log(result);
//     res.status(200).json({message: "Update successful!"});
//   });
// });

// app.get('/api/posts', (req, res, next)=>{
//   //add dummy posts content.Later should get from database
//   //const posts =[

//     //To retrieve data from database
//     Post.find().then(documents => {
//       res.status(200).json({
//         message:'Post fetched sucessfully',
//         posts: documents
//       });
//     })
//     /*{
//       id:'T001',
//       title: 'First server side post',
//       content: 'This is coming from the server'
//     },
//     {
//       id:'T002',
//       title: 'Second server side post',
//       content: 'This is coming from the server'
//     }]


//   //chaining the status method to json method
//   res.status(200).json({
//     message: 'Post fetched successfully',
//     //post property that get the dummy data aboove
//     posts: posts
//   });*/
// });

// app.delete('/api/posts/:id', checkAuth,(req,res,next) => {
//   Post.deleteOne({ _id: req.params.id}).then(result => {
//     console.log(result);
//     res.status(200).json({message:"Post deleted"});
//   })
// });

app.post('/api/user/login',(req,res,next)=>{
  let fetchedUser;
  User.findOne({email:req.body.email})
  .then(user => {
    if(!user){
      return res.status(401).json({
        message:'Auth failed'
      });
    }
    fetchedUser =user
    return bcrypt.compare(req.body.password, user.password)
  })
  .then(result=> {
    if(!result){
      return res.status(401).json({
        message:'Auth failed'
      });
    }const token = jwt.sign(
      {email:fetchedUser.email, userId: fetchedUser._id},
      "secret this should be longer",
      {expiresIn:'1h'}
    );
      res.status(200).json({
        token: token
      })

  })
  .catch(err => {
    return res.status(401).json({
      message:'Auth failed'
    });
  })
  })
//export for other module can import
module.exports = app;
